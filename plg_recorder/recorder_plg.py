
#
# Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com
#
# This file is part of EigenD.
#
# EigenD is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# EigenD is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
#

from pi import agent,atom,action,bundles,const,domain,errors,async,resource,utils,schedproxy,node,logic,files,upgrade,paths,toggle
from . import recorder_version as version,recorder_native
import piw
import picross
import os
import glob
import re

response_size = 1200

mode_stretch = 0
mode_unstretch = 1
mode_chop = 2

poly_data_initial = 10
poly_data_headroom = 5

# this polyphony and headroom is needed since a large amount of keygroups can be connected to the recorder, each with its own aux data 
# the current Alpha 3 setup connects 11 keygroups into it
poly_aux_initial = 15
poly_aux_headroom = 12


def render_list(list,offset,renderer):
    txt='['

    for n,l in enumerate(list[offset:]):
        ltxt = '' if not n else ','
        ltxt = ltxt + renderer(n+offset,l)
        if len(txt+ltxt) > response_size-1:
            return txt+']'
        txt=txt+ltxt

    return txt+']'

class TakeLibrary:
    tempdir = resource.user_resource_dir(resource.recordingstmp_dir,version='')
    permdir = resource.user_resource_dir(resource.recordings_dir,version='')

    def __init__(self,obs):
        self.__id = None
        self.__observer = obs
        self.__takes = {}

    def count(self):
        return len(self.__takes)

    def takelist(self):
        l = list(self.__takes.itervalues())
        l.sort(lambda a,b: cmp(int(a[0]),int(b[0])))
        return l

    def save(self):
        for path in glob.glob(self.temp_path('*','*')):
            cookie,desc,name = self.__split(path)
            newpath = os.path.join(self.permdir,os.path.basename(path))
            if os.path.exists(newpath):
                os.unlink(newpath)
            os.rename(path,newpath)
            self.__takes[cookie] = (cookie,desc,name,newpath,True)
        self.__observer.library_changed(0)

    def write(self,recording,cookie):
        cookie = int(cookie)
        tmppath=os.path.join(resource.get_home_dir(),'tmp')
        if not os.path.exists(resource.WC(tmppath)):
            os.mkdir(resource.WC(tmppath))
        tmpfilepath=os.path.join(tmppath,self.filename(cookie,'None'))
        if os.path.exists(resource.WC(tmpfilepath)):
            os.unlink(resource.WC(tmpfilepath))
        recording.write(tmpfilepath)
        path = self.temp_path(cookie,'None')
        if os.path.exists(path):
            os.unlink(path)
        os.rename(resource.WC(tmpfilepath),path)
        self.__takes[cookie] = (cookie,self.__describe(cookie,path),'None',path,False)
        self.__observer.library_added(cookie)
    
    def rename(self,cookie,name):
        cookie = int(cookie)
        bits = self.__takes.get(cookie)
        if bits is None: return
        cookie,desc,oname,path,perm = bits
        newpath = self.perm_path(cookie,name)
        print 'rename',path,'to',newpath
        os.rename(path,newpath)
        self.__takes[cookie] = (cookie,desc,name,newpath,True)
        self.__observer.library_changed(cookie)

    def delete_temp(self,cookie):
        cookie = int(cookie)
        bits = self.__takes.get(cookie)
        print 'delete temp',cookie,bits
        if bits and not bits[4]:
            self.delete(cookie)

    def delete(self,cookie):
        cookie = int(cookie)
        path = self.cookie2file(cookie)
        os.unlink(path)
        if cookie in self.__takes:
            del self.__takes[cookie]
            self.__observer.library_deleted(cookie)

    def nextid(self):
        return str(max([0]+self.__takes.keys())+1)

    def add_file(self,path):
        cookie,desc,name = self.__split(path)
        self.__takes[cookie] = (cookie,desc,name,path,True)
        self.__observer.library_added(cookie)

    def filename(self,cookie,name):
        return '%s-%s-current-%s' % (cookie,name,self.__id)

    def temp_path(self,cookie,name):
        return os.path.join(self.tempdir,self.filename(cookie,name))

    def perm_path(self,cookie,name):
        return os.path.join(self.permdir,self.filename(cookie,name))

    def setup(self,id):
        id = id.replace('<','').replace('>','').replace(':','_').replace('/','_')
        print 'setup',id
        self.__id = id
        for path in glob.glob(self.temp_path('*','*')):
            os.remove(path)
        for path in glob.glob(self.perm_path('*','*')):
            self.add_file(path)

    def name2cookie(self,name):
        for c,d,n,f,p in self.__takes.itervalues():
            if n==name:
                return str(c)
        return None

    def cookie2file(self,cookie):
        cookie = int(cookie)
        bits = self.__takes.get(cookie)
        return bits[3] if bits else None

    def cookie2name(self,cookie):
        cookie = int(cookie)
        bits = self.__takes.get(cookie)
        return bits[2] if bits else None

    def getcookie(self,name):
        c = self.name2cookie(name)
        if c:
            return c
        try:
            return name if int(name) in self.__takes else None
        except:
            return None

    def __describe(self,cookie,file):
        cookie = int(cookie)
        r = recorder_native.read_meta(file)
        start = r.get_tag(2)
        dur = r.get_tag(4)
        return 'take %d: start: %0.2f duration: %0.2f' % (cookie, start.as_float() if start.is_float() else 0, dur.as_float() if dur.is_float() else 0)

    def __split(self,path):
        cookie,name,_,id = os.path.basename(path).split('-')
        cookie = int(cookie)
        return cookie,self.__describe(cookie,path),name


class TakeBrowser(atom.Atom):
    def __init__(self,agent):
        self.agent = agent
        atom.Atom.__init__(self,names='take',protocols='virtual browse bind')

        self.__current = None
        self.__selected=None

        self.__timestamp = piw.tsd_time()
        self.update()

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def rpc_displayname(self,arg):
        a=self.agent.get_description()
        return a + ' takes'

    def rpc_setselected(self,arg):
        (path,selected)=logic.parse_clause(arg)
        print 'Take:setselected',selected    
        self.__selected=str(selected)
    
    def rpc_activated(self,arg):
        (path,selected)=logic.parse_clause(arg)
        print 'Take:activated',selected    
        self.agent.activate(str(selected))
        return logic.render_term(('',''))

    def resolve_name(self,name):
        print 'take:resolve_name(%s)' % name
        if name=='selection':
            name=self.__selected
            print 'name=selected',self.__selected
        c = self.agent.library.name2cookie(name)
        if c is None:
            return '[]'
        return '[%s]' % self.ideal(c)

    def rpc_current(self,arg):
        return '[]'

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)
        o = str(o)
        f = self.agent.library.cookie2file(o)
        if f is None: return '[]'
        return '[%s]' % self.ideal(o)

    def ideal(self,cookie):
        f = files.FileSystemFile(self.agent.library.cookie2file(cookie),cookie)
        f = files.get_ideal(logic.make_expansion('~server'),cookie,f,True)
        return 'ideal([~server,take],%s)' % f

    def rpc_fideal(self,arg):
        (path,cookie) = logic.parse_clause(arg)
        f = self.ideal(cookie)
        return f

    def rpc_enumerate(self,a):
        r = logic.render_term((self.agent.library.count(),0))
        return r

    def rpc_cinfo(self,a):
        return '[]'

    def rpc_finfo(self,a):
        (path,idx) = logic.parse_clause(a)
        t=self.agent.library.takelist()
        return render_list(t,idx,lambda i,t: logic.render_term((t[0],t[1],t[2])))


class Voice(atom.Atom):
    def __init__(self,agent):
        atom.Atom.__init__(self,names='voice',protocols='virtual browse bind')

        self.__agent = agent
        self.__timestamp = piw.tsd_time()
        self.__voices = []
        self.update()

    def rpc_current(self,arg):
        return '[]'

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))
        self.__voices = [ (v,n) for (v,n) in self.__agent.get_voices() ]

    def rpc_resolve(self,arg):
        return '[]'

    def rpc_cinfo(self,a):
        #print 'cinfo',a
        return '[]'

    def rpc_enumerate(self,a):
        #print 'enum',a
        s = logic.render_term((len(self.__voices),0))
        print 'voice enumerate:',s
        return s

    def rpc_finfo(self,a):
        #print 'finfo',a
        (path,idx) = logic.parse_clause(a)
        voices = [ (str(i+1),"Take %s" % v,n) for (i,(v,n)) in enumerate(self.__voices) ]
        s = render_list(voices,idx,lambda i,t: logic.render_term((t[0],t[1],t[2])))
        print 'voice finfo',a,s
        return s

    def rpc_fideal(self,arg):
        (path,cookie) = logic.parse_clause(arg)
        s = 'ideal([~server,voice],%s)' % cookie
        return s

def tsm(arg):
    if not arg:
        return {}
    return action.timespec_map(arg)

# these are defined in rec_recorder.h
record_ok=0
record_err_no_clock=-1
record_err_in_prog=-2

class Recorder(recorder_native.recorder):
    def __init__(self,clk_domain,signals):
        recorder_native.recorder.__init__(self,clk_domain,signals)
        self.__started = None
        self.__done = None

    def record_start(self, duration):
        if self.__started:
            r = self.__started
            self.__started = None
            self.abort()
            r.failed('starting aborted')

        if self.__done:
            r = self.__done
            self.__done = None
            self.abort()
            r.failed('recording aborted')

        self.__started = async.Deferred()
        self.__done = async.Deferred()

        rc = self.record(duration)
        if rc != record_ok:
            return rc,None,None

        return rc,self.__started,self.__done

    def record_started(self,recording):
        if self.__started:
            r = self.__started
            self.__started = None
            r.succeeded(recording)
        
    def record_done(self,recording):
        if self.__done:
            r = self.__done
            self.__done = None
            r.succeeded(recording)

    def record_aborted(self):
        if self.__done:
            r = self.__done
            self.__done = None
            r.failed('recording aborted')

def name_subst(name,find,repl):
    oname = []
    for w in name.split():
        if w == find: w = repl
        oname.append(w)
    return ' '.join(oname)

class RecorderOutput(bundles.Output):
    def __init__(self,linked,callback,*args,**kwds):
        self.__linked = linked
        self.__callback = callback
        bundles.Output.__init__(self,*args,**kwds)

    def property_change(self,key,value,delegate):
        if key == 'name':
            if value is not None and value.is_string():
                value = piw.makestring(name_subst(value.as_string(),'output','input'),0)
            self.__callback(self.__linked,True,value)

        if key == 'ordinal':
            self.__callback(self.__linked,False,value)

    def callback(self,isname,value):
        if isname:
            self.set_names(value)
        else:
            self.set_ordinal(value)


class RecorderInput(atom.Atom):
    def __init__(self,linked,callback,*args,**kwds):
        self.__linked = linked
        self.__callback = callback
        atom.Atom.__init__(self,*args,**kwds)

    def callback(self,isname,value):
        self.set_property('name' if isname else 'ordinal',value,notify=False,allow_veto=False)

    def set_ordinal(self,value):
        self.__callback(self.__linked,False,value)

    def set_names(self,value):
        value = name_subst(value,'input','output')
        self.__callback(self.__linked,True,value)

    def property_veto(self,key,value):
        if atom.Atom.property_veto(self,key,value):
            return True

        return key in ['name','ordinal']

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        self.domain = piw.clockdomain_ctl()
        agent.Agent.__init__(self, signature=version, names='recorder', protocols='bind',container=(3,'agent',atom.VerbContainer(clock_domain=self.domain)),ordinal=ordinal)

        def output_link(ordinal,*args,**kwds):
            self[2][ordinal].callback(*args,**kwds)

        def input_link(ordinal,*args,**kwds):
            self[1][ordinal].callback(*args,**kwds)


        self[2] = atom.Atom(names='outputs')

        self[2][2] = RecorderOutput(2,input_link,2,False,names='pressure output', protocols='')
        self[2][3] = RecorderOutput(3,input_link,3,False,names='roll output', protocols='')
        self[2][4] = RecorderOutput(4,input_link,4,False,names='yaw output', protocols='')
        self[2][15] = RecorderOutput(19,input_link,5,False,names='key output', protocols='')

        self[2][5] = RecorderOutput(5,input_link,1,False,names='auxilliary output', ordinal=1, protocols='')
        self[2][6] = RecorderOutput(6,input_link,1,False,names='auxilliary output', ordinal=2, protocols='')
        self[2][7] = RecorderOutput(7,input_link,1,False,names='auxilliary output', ordinal=3, protocols='')
        self[2][8] = RecorderOutput(8,input_link,1,False,names='auxilliary output', ordinal=4, protocols='')
        self[2][9] = RecorderOutput(9,input_link,1,False,names='auxilliary output', ordinal=5, protocols='')
        self[2][10] = RecorderOutput(10,input_link,1,False,names='auxilliary output', ordinal=6, protocols='')
        self[2][11] = RecorderOutput(11,input_link,1,False,names='auxilliary output', ordinal=7, protocols='')
        self[2][12] = RecorderOutput(12,input_link,1,False,names='auxilliary output', ordinal=8, protocols='')
        self[2][13] = RecorderOutput(13,input_link,1,False,names='auxilliary output', ordinal=9, protocols='')
        self[2][14] = RecorderOutput(14,input_link,1,False,names='auxilliary output', ordinal=10, protocols='')
        self[2][16] = RecorderOutput(15,input_link,1,False,names='controller output', protocols='')

        self.output_data = bundles.Splitter(self.domain,self[2][2],self[2][3],self[2][4],self[2][15])
        self.output_aux1 = bundles.Splitter(self.domain,self[2][5])
        self.output_aux2 = bundles.Splitter(self.domain,self[2][6])
        self.output_aux3 = bundles.Splitter(self.domain,self[2][7])
        self.output_aux4 = bundles.Splitter(self.domain,self[2][8])
        self.output_aux5 = bundles.Splitter(self.domain,self[2][9])
        self.output_aux6 = bundles.Splitter(self.domain,self[2][10])
        self.output_aux7 = bundles.Splitter(self.domain,self[2][11])
        self.output_aux8 = bundles.Splitter(self.domain,self[2][12])
        self.output_aux9 = bundles.Splitter(self.domain,self[2][13])
        self.output_aux10 = bundles.Splitter(self.domain,self[2][14])
        self.output_controller = bundles.Splitter(self.domain,self[2][16])

        self.poly_data = piw.polyctl(poly_data_initial,self.output_data.cookie(),False,poly_data_headroom)
        self.poly_aux1 = piw.polyctl(poly_aux_initial,self.output_aux1.cookie(),False,poly_aux_headroom)
        self.poly_aux2 = piw.polyctl(poly_aux_initial,self.output_aux2.cookie(),False,poly_aux_headroom)
        self.poly_aux3 = piw.polyctl(poly_aux_initial,self.output_aux3.cookie(),False,poly_aux_headroom)
        self.poly_aux4 = piw.polyctl(poly_aux_initial,self.output_aux4.cookie(),False,poly_aux_headroom)
        self.poly_aux5 = piw.polyctl(poly_aux_initial,self.output_aux5.cookie(),False,poly_aux_headroom)
        self.poly_aux6 = piw.polyctl(poly_aux_initial,self.output_aux6.cookie(),False,poly_aux_headroom)
        self.poly_aux7 = piw.polyctl(poly_aux_initial,self.output_aux7.cookie(),False,poly_aux_headroom)
        self.poly_aux8 = piw.polyctl(poly_aux_initial,self.output_aux8.cookie(),False,poly_aux_headroom)
        self.poly_aux9 = piw.polyctl(poly_aux_initial,self.output_aux9.cookie(),False,poly_aux_headroom)
        self.poly_aux10 = piw.polyctl(poly_aux_initial,self.output_aux10.cookie(),False,poly_aux_headroom)
        self.poly_controller = piw.polyctl(poly_aux_initial,self.output_controller.cookie(),False,poly_aux_headroom)

        self.output_clone = piw.clone(True)
        self.output_clone.set_filtered_output(2,self.poly_data.cookie(),piw.event_deaggregation_filter2(2,100))
        self.output_clone.set_filtered_output(3,self.poly_aux1.cookie(),piw.event_deaggregation_filter(3))
        self.output_clone.set_filtered_output(4,self.poly_aux2.cookie(),piw.event_deaggregation_filter(4))
        self.output_clone.set_filtered_output(5,self.poly_aux3.cookie(),piw.event_deaggregation_filter(5))
        self.output_clone.set_filtered_output(6,self.poly_aux4.cookie(),piw.event_deaggregation_filter(6))
        self.output_clone.set_filtered_output(7,self.poly_aux5.cookie(),piw.event_deaggregation_filter(7))
        self.output_clone.set_filtered_output(8,self.poly_aux6.cookie(),piw.event_deaggregation_filter(8))
        self.output_clone.set_filtered_output(9,self.poly_aux7.cookie(),piw.event_deaggregation_filter(9))
        self.output_clone.set_filtered_output(10,self.poly_aux8.cookie(),piw.event_deaggregation_filter(10))
        self.output_clone.set_filtered_output(11,self.poly_aux9.cookie(),piw.event_deaggregation_filter(11))
        self.output_clone.set_filtered_output(12,self.poly_aux10.cookie(),piw.event_deaggregation_filter(12))
        self.output_clone.set_filtered_output(13,self.poly_controller.cookie(),piw.event_deaggregation_filter(13))

        self.output_aggregator = piw.aggregator(self.output_clone.cookie(), self.domain)

        self.player = recorder_native.kplayer(self.domain, 16, 9, self.output_aggregator.get_output(2))
        self.recorder = Recorder(self.domain,9)

        self.input_clone = piw.clone(True)
        self.input_clone.set_output(1,self.output_aggregator.get_output(1))
        self.input_clone.set_output(2,self.player.cookie())
        self.input_clone.set_output(3,self.recorder.cookie())

        self.input_aggregator = piw.aggregator(self.input_clone.cookie(), self.domain)

        self.input_clock = bundles.ScalarInput(self.input_aggregator.get_filtered_output(1,piw.event_aggregation_filter(1)), self.domain, signals=(1,2,3))
        self.input_clock.add_upstream(self.verb_container().clock)

        self.input_poly = piw.polyctl(10, self.input_aggregator.get_filtered_output(2,piw.event_aggregation_filter(2)), False, 5)
        self.input_data = bundles.VectorInput(self.input_poly.cookie(), self.domain, signals=(2,3,4,5))

        self.input_aux1 = bundles.VectorInput(self.input_aggregator.get_filtered_output(3,piw.event_aggregation_filter(3)), self.domain, signals=(1,))
        self.input_aux2 = bundles.VectorInput(self.input_aggregator.get_filtered_output(4,piw.event_aggregation_filter(4)), self.domain, signals=(1,))
        self.input_aux3 = bundles.VectorInput(self.input_aggregator.get_filtered_output(5,piw.event_aggregation_filter(5)), self.domain, signals=(1,))
        self.input_aux4 = bundles.VectorInput(self.input_aggregator.get_filtered_output(6,piw.event_aggregation_filter(6)), self.domain, signals=(1,))
        self.input_aux5 = bundles.VectorInput(self.input_aggregator.get_filtered_output(7,piw.event_aggregation_filter(7)), self.domain, signals=(1,))
        self.input_aux6 = bundles.VectorInput(self.input_aggregator.get_filtered_output(8,piw.event_aggregation_filter(8)), self.domain, signals=(1,))
        self.input_aux7 = bundles.VectorInput(self.input_aggregator.get_filtered_output(9,piw.event_aggregation_filter(9)), self.domain, signals=(1,))
        self.input_aux8 = bundles.VectorInput(self.input_aggregator.get_filtered_output(10,piw.event_aggregation_filter(10)), self.domain, signals=(1,))
        self.input_aux9 = bundles.VectorInput(self.input_aggregator.get_filtered_output(11,piw.event_aggregation_filter(11)), self.domain, signals=(1,))
        self.input_aux10 = bundles.VectorInput(self.input_aggregator.get_filtered_output(12,piw.event_aggregation_filter(12)), self.domain, signals=(1,))
        self.input_controller = bundles.VectorInput(self.input_aggregator.get_filtered_output(13,piw.event_aggregation_filter(13)), self.domain, signals=(1,))

        self[1] = atom.Atom(names='inputs')

        self[1][2]=RecorderInput(2,output_link,domain=domain.BoundedFloat(0,1), policy=self.input_data.vector_policy(2,False),names='pressure input')
        self[1][3]=RecorderInput(3,output_link,domain=domain.BoundedFloat(-1,1), policy=self.input_data.vector_policy(3,False),names='roll input')
        self[1][4]=RecorderInput(4,output_link,domain=domain.BoundedFloat(-1,1), policy=self.input_data.vector_policy(4,False),names='yaw input')
        self[1][19]=RecorderInput(15,output_link,domain=domain.Aniso(), policy=self.input_data.vector_policy(5,False),names='key input')

        self[1][5]=RecorderInput(5,output_link,domain=domain.Aniso(), policy=self.input_aux1.vector_policy(1,False),names='auxilliary input', ordinal=1)
        self[1][6]=RecorderInput(6,output_link,domain=domain.Aniso(), policy=self.input_aux2.vector_policy(1,False),names='auxilliary input', ordinal=2)
        self[1][7]=RecorderInput(7,output_link,domain=domain.Aniso(), policy=self.input_aux3.vector_policy(1,False),names='auxilliary input', ordinal=3)
        self[1][8]=RecorderInput(8,output_link,domain=domain.Aniso(), policy=self.input_aux4.vector_policy(1,False),names='auxilliary input', ordinal=4)
        self[1][9]=RecorderInput(9,output_link,domain=domain.Aniso(), policy=self.input_aux5.vector_policy(1,False),names='auxilliary input', ordinal=5)
        self[1][10]=RecorderInput(10,output_link,domain=domain.Aniso(), policy=self.input_aux6.vector_policy(1,False),names='auxilliary input', ordinal=6)
        self[1][11]=RecorderInput(11,output_link,domain=domain.Aniso(), policy=self.input_aux7.vector_policy(1,False),names='auxilliary input', ordinal=7)
        self[1][12]=RecorderInput(12,output_link,domain=domain.Aniso(), policy=self.input_aux8.vector_policy(1,False),names='auxilliary input', ordinal=8)
        self[1][13]=RecorderInput(13,output_link,domain=domain.Aniso(), policy=self.input_aux9.vector_policy(1,False),names='auxilliary input', ordinal=9)
        self[1][14]=RecorderInput(14,output_link,domain=domain.Aniso(), policy=self.input_aux10.vector_policy(1,False),names='auxilliary input', ordinal=10)
        self[1][15]=RecorderInput(16,output_link,domain=domain.Aniso(), policy=self.input_controller.vector_policy(1,False),names='controller input')

        self[1][16]=atom.Atom(domain=domain.BoundedFloat(0,10000000), policy=self.input_clock.nodefault_policy(1,False),names='song beat input')
        self[1][17]=atom.Atom(domain=domain.BoundedFloat(0,100), policy=self.input_clock.nodefault_policy(2,False),names='bar beat input')
        self[1][18]=atom.Atom(domain=domain.Bool(), init=False, policy=self.input_clock.nodefault_policy(3,False),names='running input')

        c = dict(c=schedproxy.get_constraints(),
                 x='[ideal([~server,take]),singular]',
                 o='option(with,[or([[matches([stretch],stretch)],[matches([un,stretch],unstretch)],[matches([chop],chop)]])])'
            )

        a = dict(create_action=self.__create_deferred, destroy_action=self.__destroy_deferred,clock=True)

        self.add_verb2(1,'record([],None,role(None,[mass([bar])]),%(o)s)'%c,self.__record)
        self.add_verb2(2,'play([un],None,option(None,%(x)s))'%c,self.__unplay)
        self.add_verb2(3,'play([],None,role(None,%(x)s),%(o)s)'%c,callback=self.__play_now,**a)
        self.add_verb2(4,'play([],None,role(None,%(x)s),role(at,%(c)s),option(until,%(c)s),option(every,%(c)s),%(o)s)'%c,callback=self.__play_aue,**a)
        self.add_verb2(5,'play([],None,role(None,%(x)s),role(until,%(c)s),option(every,%(c)s),%(o)s)'%c,callback=self.__play_ue,**a)
        self.add_verb2(6,'play([],None,role(None,%(x)s),role(every,%(c)s),%(o)s)'%c,callback=self.__play_e,**a)
        self.add_verb2(7,'play([dont],None,option(None,%(x)s))'%c,self.__unplay)
        self.add_verb2(8,'repeat([],None,role(None,%(x)s),%(o)s)'%c,callback=self.__repeat,**a)
        self.add_verb2(9,'name([],None,role(None,[ideal([~server,take]),singular]),role(to,[abstract]))',self.__name)
        self.add_verb2(10,'copy([],None,role(None,[gideal(take),singular]))',callback=self.__copy)
        self.add_verb2(12,'delete([],None,role(None,[ideal([~server,take]),singular]))',self.__delete)

        self.add_verb2(15,'play([toggle],None,role(None,%(x)s),%(o)s)'%c,callback=self.__tog_play_now,**a)
        self.add_verb2(16,'play([toggle],None,role(None,%(x)s),role(at,%(c)s),option(until,%(c)s),option(every,%(c)s),%(o)s)'%c,callback=self.__tog_play_aue,**a)
        self.add_verb2(17,'play([toggle],None,role(None,%(x)s),role(until,%(c)s),option(every,%(c)s),%(o)s)'%c,callback=self.__tog_play_ue,**a)
        self.add_verb2(18,'play([toggle],None,role(None,%(x)s),role(every,%(c)s),%(o)s)'%c,callback=self.__tog_play_e,**a)
        self.add_verb2(19,'repeat([toggle],None,role(None,%(x)s),%(o)s)'%c,callback=self.__tog_repeat,**a)

        self.add_verb2(20,'cancel([],None)',self.__cancel)

        self[4] = TakeBrowser(self)
        self.library = TakeLibrary(self)

        self.__scheduler = schedproxy.SchedProxy(delegate=self)
        self.__schedready = False

        self[5] = Voice(self)

        # don't use self[6], it was previously used by a legacy status output that was removed in 1.2

        self[7]=toggle.Toggle(None,self.domain,container=(None,'overdub',self.verb_container()),init=False,names='overdub',transient=True)
        self[8]=atom.Atom(domain=domain.String(),names='scheduler identifier',policy=atom.default_policy(self.__schedchange),transient=True)

    def __schedchange(self,arg):
        self.__scheduler.set_address(arg)

    def __is_overdub_enabled(self):
        return self[7].get_value()

    def server_opened(self):
        agent.Agent.server_opened(self)
        self.library.setup(paths.to_absolute(self.id()))

    def library_changed(self,c):
        self[4].update()

    def library_added(self,c):
        self[4].update()

    def library_deleted(self,c):
        self[4].update()

    def agent_presave(self,filename):
        self.library.save()

    def rpc_resolve_ideal(self,arg):
        (typ,name) = action.unmarshal(arg)
        if typ != 'take':
            return '[]'
        name = ' '.join(name)
        return self[4].resolve_name(name)

    def cookie_arg(self,cookie):
        return logic.make_term('cookie',cookie)

    def scheduler_ready(self):
        self.__schedready = True
        print 'attached to scheduler'

    def scheduler_gone(self):
        self.__schedready = False
        print 'detached from scheduler'

    def __record(self,subject,duration,mode):

        duration = action.mass_quantity(duration)
        mode = self.__crack_mode(mode)

        if not self.__schedready:
            print 'no scheduler'
            return async.success(errors.state_error1('scheduler','use'))

        result,started,done = self.recorder.record_start(duration)

        if result==record_err_no_clock:
            print 'metronome isnt started'
            return async.success(errors.state_error1('metronome','start'))

        if result==record_err_in_prog:
            print 'recording already in progress'
            return async.success(errors.message('recording already in progress'))

        id = self.library.nextid()
        if not self.__is_overdub_enabled():
            self.__unplay_auto()

        print 'recording',duration,'bars',id

        def trigger_ok(r,trigger):
            cookie = self.player.load(id,r,0)
            event,status = self.verb_defer(3,(True,trigger,2,mode),trigger,None,self.cookie_arg(id))
            self.player.unload(cookie,False)
            self[5].update()
            print 'trigger created',trigger,'cookie is', cookie

        def trigger_failed(msg):
            print 'cannot create trigger',msg

        def started_ok(r):
            start = r.get_tag(3).as_float()
            duration = r.get_tag(4).as_float()
            desc = "take %s at song beat %g every %g song beat" % (id,start,duration)
            self.library.write(recorder_native.recording(),id)
            trigger = self.__scheduler.create_trigger('schema("%s",[m(3,%f,%f)])' % (desc,duration,start%duration))
            trigger.setCallback(trigger_ok,r).setErrback(trigger_failed)
            print 'record started: start=',start,'duration=',duration

        def started_failed(msg):
            print 'record failed:',msg
            
        def done_ok(r):
            print 'record completed: signals=',r.signals(),'wires=',r.wires(),'start=',r.get_tag(2)
            self.library.write(r,id)

        def done_failed(msg):
            print 'record failed:',msg
            self.__unplay_auto()
            
        done.setCallback(done_ok).setErrback(done_failed)
        started.setCallback(started_ok).setErrback(started_failed)

        return action.nosync_return()

    def __cancel(self,*args):
        self.recorder.abort()
        return action.nosync_return()

    def __unplay_auto(self):
        for (id,ctx,cookie) in self.verb_events(lambda l,i: i==3):
            (autor,trigger,poly,mode) = ctx
            if autor:
                #print 'tear down',id,trigger,cookie
                self.__scheduler.delete_trigger(trigger)
                self.library.delete_temp(self.player.cookiename(cookie))
                self.verb_cancel(id)
        self[5].update()

    def get_voices(self):
        for (id,ctx,cookie) in self.verb_events(lambda l,i: i==3):
            print id,ctx,cookie
            take = self.player.cookiename(cookie)
            name = self.library.cookie2name(take)
            yield (take,name)

    def __unplay(self,subj,arg):
        #print 'unplaying',arg

        if arg is None:
            latest_cookie=0
            latest_ctx=None
            latest_id=None

            for (id,ctx,cookie) in self.verb_events(lambda l,i: i==3):
                if cookie>latest_cookie:
                    latest_cookie=cookie
                    latest_ctx=ctx
                    latest_id=id

            if latest_cookie:
                (autor,trigger,poly,mode) = latest_ctx
                self.__scheduler.delete_trigger(trigger)
                self.verb_cancel(latest_id)
                self[5].update()
                self.player.abortcookie(latest_cookie)
                #self.set_all_status(const.status_inactive)

            return action.nosync_return()

        arg = self.__crack_arg(arg)

        if arg:
            c = self.player.getcookie(arg)

            while c:
                #print 'clearing cookie',c

                for (id,ctx,cookie) in self.verb_events(lambda l,i: i==3):
                    if c==cookie:
                        (autor,trigger,poly,mode) = ctx
                        #print 'tear down',id,trigger,cookie
                        self.__scheduler.delete_trigger(trigger)
                        self.verb_cancel(id)
                        self[5].update()

                c = self.player.getcookie(arg)
            self.player.abort(arg)

        return action.nosync_return()

    @utils.nothrow
    def __create_deferred(self,ctx,subj,arg):
        #print 'create deferred',arg
        arg = self.__crack_arg(arg)
        #print '->',arg

        if ctx:
            (autor,trigger,poly,mode) = ctx
        else:
            (autor,trigger,poly,mode) = (False,None,1,mode_stretch)

        cookie = self.player.clonecookie(arg,poly)

        if not cookie:
            cookie = self.__load(arg,poly)

        if not cookie:
            return None

        return (self.player.player(cookie,mode),cookie)

    def __destroy_deferred(self,ctx,cookie):
        #print 'unloading cookie',cookie
        self.player.unload(cookie,True)

    def activate(self,selected):
        self.__do_play_now(selected,mode_stretch)

    def playing(self,arg):
        arg = self.__crack_arg(arg)
        c = self.player.getcookie(arg)
        for (id,ctx,cookie) in self.verb_events(lambda l,i: i==3):
             if c==cookie:
                print 'matching cookie',c,' found' 
                return True 
        print 'cookie',c,'not found'
        return False

    def __tog_play_now(self,subj,arg,mode):
        print '__tog_play_now'
        if self.playing(arg):
            return self.__unplay(subj,arg)
        else:
            return self.__play_now(subj,arg,mode)

    def __play_now(self,subj,arg,mode):
        arg = self.__crack_arg(arg)
        mode = self.__crack_mode(mode)
        return self.__do_play_now(arg,mode)

    def __do_play_now(self,arg,mode):
        cookie = self.__load(arg,1)

        if not cookie:
            thing= 'take %s' %str(arg)
            return async.success(errors.invalid_thing(thing,'play'))

        f = self.player.player(cookie,mode)
        ff = piw.fastchange(f)
        ff(piw.makefloat_bounded(1,0,0,1,0))
        self.player.unload(cookie,False)
        return action.nosync_return()

    def __crack_arg(self,arg):
        if logic.is_pred_arity(arg,'cookie',1,1):
            return str(arg.args[0])

        type,thing = action.crack_ideal(action.arg_objects(arg)[0])
        cookie = str(thing.args[1][2])
        print 'take',arg,'cookie',cookie
        return cookie

    def __crack_mode(self,mode):
        if mode:
            mode = action.abstract_string(mode)
            if mode == 'stretch': return mode_stretch
            if mode == 'unstretch': return mode_unstretch
            if mode == 'chop': return mode_chop
        return mode_stretch

    def __tog_repeat(self,subj,arg,mode):
        print '__tog_repeat',arg
        t = self.__crack_arg(arg)
        if self.playing(arg):
            print 'playing: call unplay'
            return self.__unplay(subj,arg)
        else:
            print 'not playing: call repeat'
            return self.__repeat(subj,arg,mode)

    def __repeat(self,subj,arg,mode):
        arg = self.__crack_arg(arg)
        mode = self.__crack_mode(mode)

        print 'repeat mode',mode

        if not self.__schedready:
            return async.success(errors.state_error1('scheduler','use'))

        path = self.library.cookie2file(arg)
        if not path or not os.path.exists(path):
            thing='take %s' % str(arg)
            return async.success(errors.invalid_thing(thing,'repeat'))

        recording = recorder_native.read(path)
        cookie = self.player.load(arg,recording,1)
        delta = recording.get_tag(5).as_float()
        start = recording.get_tag(3).as_float()
        duration = recording.get_tag(4).as_float()
        desc = "take %s every %g song beat" % (arg,duration)
        schema = 'schema("%s",[m(3,%f,%f)])' % (desc,duration,start%duration)

        r = async.Deferred()

        def trigger_ok(trigger):
            print '__repeat:trigger_ok'
            event,status = self.verb_defer(3,(False,trigger,4,mode),trigger,None,self.cookie_arg(arg))
            self[5].update()
            self.player.unload(cookie,False)
            r.succeeded(action.nosync_return())

        def trigger_failed(msg):
            r.failed(msg)

        trigger = self.__scheduler.create_trigger(schema)
        trigger.setCallback(trigger_ok).setErrback(trigger_failed)

        return r

    def __tog_play_aue(self,subj,arg,a,u,e,mode):
        print '__tog_play_aue'
        if self.playing(arg):
            return self.__unplay(subj,arg)
        else:
            return self.__play_aue(subj,arg,a,u,e,mode)

    def __play_aue(self,subj,arg,a,u,e,mode):
        #print 'play aue',arg
        arg = self.__crack_arg(arg)
        mode = self.__crack_mode(mode)
        #print '->',arg

        a = tsm(a)
        u = tsm(u)
        e = tsm(e)

        if not self.__schedready:
            return async.success(errors.state_error1('scheduler','use'))

        path = self.library.cookie2file(arg)
        if not path or not os.path.exists(path):
            thing='take %s' %str(arg)
            return async.success(errors.invalid_thing(thing,'play'))
        recording = recorder_native.read(path)
        cookie = self.player.load(arg,recording,1)
        delta = recording.get_tag(5).as_float()
        schema = schedproxy.make_schema(a,u,e,delta,prefix='take %s' % arg)

        #print 'playback',arg,'delta=',delta,'schema',schema
        r = async.Deferred()

        def trigger_ok(trigger):
            print '__play_aue:trigger_ok'
            event,status = self.verb_defer(3,(False,trigger,4,mode),trigger,None,self.cookie_arg(arg))
            self.player.unload(cookie,False)
            r.succeeded(action.nosync_return())

        def trigger_failed(msg):
            r.failed(msg)

        trigger = self.__scheduler.create_trigger(schema)
        trigger.setCallback(trigger_ok).setErrback(trigger_failed)

        return r

    def __tog_play_ue(self,subj,arg,u,e,m):
        print '__tog_play_ue'
        if self.playing(arg):
            return self.__unplay(subj,arg)
        else:
            return self.__play_ue(subj,arg,u,e,m)

    def __play_ue(self,subj,arg,u,e,m):
        return self.__play_aue(subj,arg,None,u,e,m)

    def __tog_play_e(self,subj,arg,e,m):
        print '__tog_play_e'
        if self.playing(arg):
            return self.__unplay(subj,arg)
        else:
            return self.__play_e(subj,arg,e,m)

    def __play_e(self,subj,arg,e,m):
        return self.__play_aue(subj,arg,None,None,e,m)

    def resolve_file_cookie(self,cookie):
        path = self.library.cookie2file(cookie)
        #print cookie,'->',path

        if path:
            return files.FileSystemFile(path,cookie)

        return agent.Agent.resolve_file_cookie(self,cookie)

    def __load(self,id,poly):
        path = self.library.cookie2file(id)
        #print 'path is',path

        if path and os.path.exists(path):
            #print 'loading',id,'from',path,'poly',poly
            r = recorder_native.read(path)
            return self.player.load(id,r,poly)

        print 'no recording named',id
        return None

    def __name(self,subj,thing,name):
        thing = self.__crack_arg(thing)
        name = action.abstract_string(name)
        try:
            self.library.rename(thing,name)
            self[5].update()
            return action.nosync_return()
        except:
            utils.log_exception()
            return async.success(errors.invalid_value(thing,'name'))

    @async.coroutine('internal error')
    def __copy(self,sub,take):
        type,thing = action.crack_ideal(action.arg_objects(take)[0])
        id = self.library.nextid()
        fn = self.library.perm_path(id,None)

        print 'copy',type,thing,'to',self.id(),fn

        r = files.copy_file(logic.render_term(thing),fn)
        yield r

        if not r.status():
            yield async.Coroutine.failure(*r.args())

        print 'transfer complete'

        self.library.add_file(fn)
        self[5].update()
        yield async.Coroutine.success(action.nosync_return())

    def __delete(self,subj,thing):
        self.__unplay(subj,thing)
        cookie = self.__crack_arg(thing)
        try:
            self.library.delete(cookie)
            return action.nosync_return()
        except:
            return async.success(errors.invalid_value(cookie,'delete'))

agent.main(Agent)
