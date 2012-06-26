
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

from pi import agent,atom,action,logic,bundles,domain,policy,node,resource,async,talker,collection
from . import scale_manager_version as version
import piw
import os
import ConfigParser
import shutil

from pi.logic.shortcuts import T

def scales_equal(s1,s2):
    if len(s1)!=len(s2): return False
    for (e1,e2) in zip(s1,s2):
        if abs(e1-e2)>0.001: return False
    return True

class Event(talker.Talker):
    def __init__(self,eventlist,fast,index):
        self.__index = index
        self.__eventlist = eventlist

        talker.Talker.__init__(self,self.__eventlist.agent.finder,fast,None,ordinal=index,protocols='remove')

    def __change(self,v):
        if v.is_string():
            self.get_private().set_data(v)

    def describe(self):
        return self.get_private().get_data().as_string()

class EventList(collection.Collection):
    def __init__(self,agent):
        collection.Collection.__init__(self,creator=self.__create,wrecker=self.__wreck,names='event',protocols='hidden-connection')
        self.agent = agent
        self.__event = bundles.FastSender(2)

    def __create(self,i):
        return Event(self,self.__event,i)

    def __wreck(self,k,v):
        pass

    @async.coroutine('internal error')
    def instance_create(self,name):
        e = Event(self,self.__event,name)
        self[name] = e
        e.attached()
        yield async.Coroutine.success(e)

    @async.coroutine('internal error')
    def instance_wreck(self,k,e,name):
        print 'killing event',k
        del self[k]
        r = e.clear_phrase()
        yield r
        print 'killed event',k
        yield async.Coroutine.success()

    def create_event(self,text):
        i = self.find_hole()
        e = Event(self,self.__event,i)
        self[i] = e
        e.attached()
        r = e.set_phrase(text)
        return r

    def cancel_event(self,index=None):
        for i,e in self.items():
            if not index or index==i:
                del self[i]
                e.clear_phrase()

    def activate(self):
        self.__event.trigger()


class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='scale manager',ideals='note scale',ordinal=ordinal)

        self.finder = talker.TalkerFinder()

        self[1]=VirtualNote()
        self[3]=EventList(self)
        self[2]=VirtualScale(self[3].activate)

        self.add_verb2(4,'do([],None,role(None,[abstract]),role(when,[abstract,matches(["activation"])]),option(using,[instance(~server)]))', self.__do_verb)
        self.add_verb2(1,'cancel([],None,option(None,[singular,numeric]))', self.__cancel_verb)
        self.add_verb2(5,'choose([],None,role(None,[ideal([None,scale]),singular]))',self.__choose_verb)

    def __choose_verb(self,subject,scale):
        type,thing = action.crack_ideal(action.arg_objects(scale)[0])
        print 'choose',scale,thing
        self[2].reset_to(thing)

    @async.coroutine('internal error')
    def __cancel_verb(self,subject,i):
        i = int(action.abstract_string(i)) if i else None
        self[2].reset()
        self[3].cancel_event(i)
        yield async.Coroutine.success()

    def __query(self,k,u):
        return [ v.id() for v in self[3].itervalues() ]

    @async.coroutine('internal error')
    def __do_verb(self,subject,t,k,c):
        t = action.abstract_string(t)
        r = self[3].create_event(t)
        yield r
        yield async.Coroutine.success()

    def rpc_resolve_ideal(self,arg):
        (type,name) = action.unmarshal(arg)
        name = ' '.join(name)

        if type=='note':
            return self[1].resolve_name(name)

        if type=='scale':
            return self[2].resolve_name(name)

        return '[]'

class VirtualNote(atom.Atom):
    values=['notec','notecsharp','noted','notedsharp','notee','notef','notefsharp','noteg','notegsharp','notea','noteasharp','noteb']

    def __init__(self):
        atom.Atom.__init__(self,names='note',protocols='virtual')

    def resolve_name(self,name):
        try:
            return '[%s]' % self.__ideal(self.values.index(name))

        except ValueError:
            return '[]'

    def rpc_resolve(self,a):
        (a,o) = logic.parse_clause(a)

        if a or not o:
            return '[]'

        o = int(o)-1
        if o>=0 and o<len(self.values):
            return '[%s]' % self.__ideal(o)

        return '[]'

    def __ideal(self,o):
        return 'ideal([none,note],%s)' % o


class VirtualScale(atom.Atom):
    init_values=(
        ('natural major', [0, 2, 4, 5, 7, 9, 11, 12]) ,
        ('ionian', [0, 2, 4, 5, 7, 9, 11, 12]) ,
        ('major', [0, 2, 4, 5, 7, 9, 11, 12]) ,
        ('chromatic', [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]) ,
        ('spanish 8 tone', [0, 1, 3, 4, 5, 6, 8, 10, 12]) ,
        ('flamenco', [0, 1, 3, 4, 5, 7, 8, 10, 12]) ,
        ('symmetrical', [0, 1, 3, 4, 6, 7, 9, 10, 12]) ,
        ('inverted diminished', [0, 1, 3, 4, 6, 7, 9, 10, 12]) ,
        ('diminished', [0, 2, 3, 5, 6, 8, 9, 11, 12]) ,
        ('whole tone', [0, 2, 4, 6, 8, 10, 12]) ,
        ('augmented', [0, 3, 4, 7, 8, 11, 12]) ,
        ('3 semitone', [0, 3, 6, 9, 12]) ,
        ('4 semitone', [0, 4, 8, 12]) ,
        ('locrian ultra', [0, 1, 3, 4, 6, 8, 9, 12]) ,
        ('locrian super', [0, 1, 3, 4, 6, 8, 10, 12]) ,
        ('indian', [0, 1, 3, 4, 7, 8, 10, 12]) ,
        ('locrian', [0, 1, 3, 5, 6, 8, 10, 12]) ,
        ('phrygian', [0, 1, 3, 5, 7, 8, 10, 12]) ,
        ('neapolitan minor', [0, 1, 3, 5, 7, 8, 11, 12]) ,
        ('javanese', [0, 1, 3, 5, 7, 9, 10, 12]) ,
        ('neapolitan major', [0, 1, 3, 5, 7, 9, 11, 12]) ,
        ('todi', [0, 1, 3, 6, 7, 8, 11, 12]) ,
        ('persian', [0, 1, 4, 5, 6, 8, 11, 12]) ,
        ('oriental', [0, 1, 4, 5, 6, 9, 10, 12]) ,
        ('phrygian major', [0, 1, 4, 5, 7, 8, 10, 12]) ,
        ('spanish', [0, 1, 4, 5, 7, 8, 10, 12]) ,
        ('jewish', [0, 1, 4, 5, 7, 8, 10, 12]) ,
        ('double harmonic', [0, 1, 4, 5, 7, 8, 11, 12]) ,
        ('gypsy', [0, 1, 4, 5, 7, 8, 11, 12]) ,
        ('byzantine', [0, 1, 4, 5, 7, 8, 11, 12]) ,
        ('chahargah', [0, 1, 4, 5, 7, 8, 11, 12]) ,
        ('marva', [0, 1, 4, 6, 7, 9, 11, 12]) ,
        ('enigmatic', [0, 1, 4, 6, 8, 10, 11, 12]) ,
        ('locrian natural', [0, 2, 3, 5, 6, 8, 10, 12]) ,
        ('natural minor', [0, 2, 3, 5, 7, 8, 10, 12]) ,
        ('minor', [0, 2, 3, 5, 7, 8, 10, 12]) ,
        ('melodic minor', [0, 2, 3, 5, 7, 9, 11, 12]) ,
        ('aeolian', [0, 2, 3, 5, 7, 8, 10, 12]) ,
        ('algerian 2', [0, 2, 3, 5, 7, 8, 10, 12]) ,
        ('hungarian minor', [0, 2, 3, 6, 7, 8, 11, 12]) ,
        ('algerian', [0, 2, 3, 6, 7, 8, 11, 12]) ,
        ('algerian 1', [0, 2, 3, 6, 7, 8, 11, 12]) ,
        ('harmonic minor', [0, 2, 3, 5, 7, 8, 11, 12]) ,
        ('mohammedan', [0, 2, 3, 5, 7, 8, 11, 12]) ,
        ('dorian', [0, 2, 3, 5, 7, 9, 10, 12]) ,
        ('hungarian gypsy', [0, 2, 3, 6, 7, 8, 11, 12]),
        ('romanian', [0, 2, 3, 6, 7, 9, 10, 12]) ,
        ('locrian major', [0, 2, 4, 5, 6, 8, 10, 12]) ,
        ('arabian', [0, 1, 4, 5, 7, 8, 11, 12]) ,
        ('hindu', [0, 2, 4, 5, 7, 8, 10, 12]) ,
        ('ethiopian', [0, 2, 4, 5, 7, 8, 11, 12]) ,
        ('mixolydian', [0, 2, 4, 5, 7, 9, 10, 12]) ,
        ('mixolydian augmented', [0, 2, 4, 5, 8, 9, 10, 12]) ,
        ('harmonic major', [0, 2, 4, 5, 8, 9, 11, 12]) ,
        ('lydian minor', [0, 2, 4, 6, 7, 8, 10, 12]) ,
        ('lydian dominant', [0, 2, 4, 6, 7, 9, 10, 12]) ,
        ('overtone', [0, 2, 4, 6, 7, 9, 10, 12]) ,
        ('lydian', [0, 2, 4, 6, 7, 9, 11, 12]) ,
        ('lydian augmented', [0, 2, 4, 6, 8, 9, 10, 12]) ,
        ('leading whole tone', [0, 2, 4, 6, 8, 10, 11, 12]) ,
        ('blues', [0, 3, 5, 6, 7, 10, 12]),
        ('hungarian major', [0, 3, 4, 6, 7, 9, 10, 12]) ,
        ('pb', [0, 1, 3, 6, 8, 12]) ,
        ('balinese', [0, 1, 3, 7, 8, 12]) ,
        ('pe', [0, 1, 3, 7, 8, 12]) ,
        ('pelog', [0, 1, 3, 7, 10, 12]) ,
        ('iwato', [0, 1, 5, 6, 10, 12]) ,
        ('japanese', [0, 1, 5, 7, 8, 12]) ,
        ('kumoi', [0, 1, 5, 7, 8, 12]) ,
        ('hirajoshi', [0, 2, 3, 7, 8, 12]) ,
        ('pa', [0, 2, 3, 7, 8, 12]) ,
        ('pd', [0, 2, 3, 7, 9, 12]) ,
        ('pentatonic major', [0, 2, 4, 7, 9, 12]) ,
        ('chinese', [0, 2, 4, 7, 9, 12]) ,
        ('chinese 1', [0, 2, 4, 7, 9, 12]) ,
        ('mongolian', [0, 2, 4, 7, 9, 12]) ,
        ('pfcg', [0, 2, 4, 7, 9, 12]) ,
        ('egyptian', [0, 2, 3, 6, 7, 8, 11, 12]) ,
        ('pentatonic minor', [0, 3, 5, 7, 10, 12]) ,
        ('chinese 2', [0, 4, 6, 7, 11, 12]) ,
        ('altered',  [0, 1, 3, 4, 6, 8, 10, 12]) ,
        ('bebop dominant',  [0, 2, 4, 5, 7, 9, 10, 11, 12]) ,
        ('bebop dominant flatnine',  [0, 1, 4, 5, 7, 9, 10, 11, 12]),
        ('bebop major',  [0, 2, 4, 5, 7, 8, 9, 11, 12]),
        ('bebop minor',  [0, 2, 3, 5, 7, 8, 9, 10, 12 ]),
        ('bebop tonic minor',  [0, 2, 3, 5, 7, 8, 9, 11, 12 ]),
        ('monotone', [0]),
    )

    def add_scale(self,line,desc=None):
        try:
            x = line.split()
            name = "user %d" % self.__usercounter
            if len(x)<2: return
            intervals = [float(s) for s in x]
            self.values.append((name,intervals))
            self.__usercounter += 1
            print 'added user scale',name,intervals
            if desc: self.descriptions[name]=desc
        except:
            print 'bad scale definition',line

    def __init__(self, activate):
        atom.Atom.__init__(self,names='scale',protocols='virtual browse')
        self.descriptions = {}
        self.__user = resource.user_resource_file(resource.scalemanager_dir,'User Scales.txt',version='')
        self.__mtime = None
        self.__selected = None
        self.__selected_name = None
        self.__activated_scale = None
        self.__activated_name = None
        self.__activated_idx = None
        self.__activate = activate
        self.__timestamp = piw.tsd_time()
        self.__usercounter = 1
        self.values = list(self.init_values)
        self.update_scales()
        self.update_timestamp()

    def reset(self):
        self.__selected = None
        self.__selected_name = None
        self.__activated_scale = None
        self.__activated_name = None
        self.__activated_idx = None
        self.update_timestamp()

    def get_user_mtime(self):
        try:
            return resource.os_path_getmtime(self.__user)
        except:
            return None

    def read_user(self):
        if not resource.os_path_exists(self.__user):
            print 'no scale file',self.__user
            fr = resource.find_release_resource('scale_manager','User Scales.txt');
            if not fr:
                print 'no factory scale file',fr
                return
            print 'copy',fr,self.__user
            resource.shutil_copyfile(fr,self.__user)

        cp = ConfigParser.ConfigParser()
        cp.read(resource.WC(self.__user))

        for s in sorted(cp.sections()):
            if not cp.has_option(s,'intervals'): 
                continue
            line = cp.get(s,'intervals')
            line = line.strip()
            self.add_scale(line,s)

    def rpc_displayname(self,arg):
        return 'scales'

    def reset_to(self,scale):
        scale = list(scale)
        for i,(k,v) in enumerate(self.values):
            if scales_equal(scale,v):
                self.__selected = v
                self.__selected_name = k
                self.__activated_idx = i
                self.__activated_name = k
                self.__activated_scale = v
                self.update_timestamp()
                print 'updated choice to',k,v
                return

        print 'no match',scale
        self.__selected = None
        self.__selected_name = None
        self.__activated_scale = None
        self.__activated_name = None
        self.__activated_idx = None
        self.update_timestamp()

    def update_timestamp(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def update_scales(self):
        try:
            mtime = self.get_user_mtime()
            if mtime and (not self.__mtime or self.__mtime != mtime):
                self.__usercounter = 1
                self.values = list(self.init_values)
                self.read_user()
                print 'updated scales',self.__mtime,mtime
                self.__mtime = mtime
            else:
                print 'not updating scale',self.__mtime,mtime
        except:
            pass

    def rpc_current(self,arg):
        if self.__activated_scale is not None and self.__activated_idx < len(self.values) and self.values[self.__activated_idx][1] == self.__activated_scale:
            return '[["%s",[]]]' % self.__activated_name
        return '[[0,[]]]'

    def rpc_setselected(self,arg):
        (path,name)=logic.parse_clause(arg) 
        # convert name to scale 
        for (n,s) in self.values: 
            if n==name: 
                self.__selected=s 
                self.__selected_name=n
    
    def rpc_activated(self,arg):
        (path,name)=logic.parse_clause(arg) 
        print 'Scale:activated',arg,name
        old_idx = self.__activated_idx
        # convert name to scale 
        for i,(n,s) in enumerate(self.values): 
            if n==name: 
                self.__activated_scale=s 
                self.__activated_name=n
                self.__activated_idx=i
                if old_idx != i:
                    self.__activate()
                    self.update_timestamp()
        return logic.render_term(('',''))

    def resolve_literal(self,name):
        words = name.split()
        numbers = []

        for w in words:
            try: numbers.append(float(w))
            except: return None

        if len(numbers)<2:
            return None

        return numbers


    def resolve_name(self,name):
        if name=='selection' and self.__selected:
             return '[%s]' % self.__ideal(self.__selected)
            
        if name=='activation' and self.__activated_scale:
             return '[%s]' % self.__ideal(self.__activated_scale)

        literal = self.resolve_literal(name)
        if literal:
            return '[%s]' % self.__ideal(literal)
            
        for (n,s) in self.values:
            if n==name:
                return '[%s]' % self.__ideal(s)

        return '[]'

    def rpc_resolve(self,a):
        (a,o) = logic.parse_clause(a)
        print a,o,self.__activated_scale

        if a or not o:
            if a==('selection',) and self.__selected: return '[%s]' % self.__ideal(self.__selected)
            if a==('activation',) and self.__activated_scale: return '[%s]' % self.__ideal(self.__activated_scale)
            return '[]'

        for (n,s) in self.values:
            if s==o:
                return '[%s]' % self.__ideal(o)

        return '[]'

    def rpc_enumerate(self,a):
        self.update_scales()
        return logic.render_term((len(self.values),0))

    def rpc_cinfo(self,a):
        return '[]'

    def __describe(self,n):
        x = self.descriptions.get(n)
        if x: return "%s (%s)" % (n,x)
        return n

    def rpc_finfo(self,a):
        (path,idx) = logic.parse_clause(a)
        map = tuple([ (n,self.__describe(n),None) for (n,s) in self.values[idx:] ])
        return logic.render_term(map)

    def rpc_fideal(self,a):
        (path,o) = logic.parse_clause(a)
        return self.__ideal(o)

    def __ideal(self,scale):
        return 'ideal([None,scale],%s)' % scale

    def rpc_dinfo(self,a):
        print 'scale __dinfo:'
        activated=self.__activated_name
        l=[]
        if activated is not None:
            print 'scale dinfo',activated
            l.append(('dinfo_id',activated))
            l.append(('Selected scale',self.__describe(activated)))
            l.append(('Definition',' '.join([str(f) for f in self.__activated_scale])))
            return logic.render_term(T('keyval',tuple(l) ))
        else:
            l.append(('dinfo_id','scale'))

            return logic.render_term(T('keyval',tuple(l) ))
 

agent.main(Agent)
