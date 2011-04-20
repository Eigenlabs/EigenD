
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

import piw
import picross
import sampler2_native

from pi import agent,atom,domain,bundles,resource,action,logic,utils,async,node,upgrade,const
from plg_sampler2 import sf2
from plg_sampler2 import sampler_oscillator_version as version

from pi.logic.shortcuts import T
import os
import glob
import sys
response_size = 1200

def render_list(list,offset,renderer):
    txt='['

    for n,l in enumerate(list[offset:]):
        ltxt = '' if not n else ','
        ltxt = ltxt + renderer(n+offset,l)
        if len(txt+ltxt) > response_size-1:
            return txt+']'
        txt=txt+ltxt

    return txt+']'

class Sample(atom.Atom):
    userdir = os.path.join(resource.user_resource_dir('Soundfont',version=''))
    reldir = os.path.join(picross.global_resource_dir(),'soundfont')

    def __init__(self,agent):
        self.__scan()
        self.agent = agent
        atom.Atom.__init__(self,names='sample',protocols='virtual browse',domain=domain.String(),policy=atom.load_policy(self.__loadcurrent))

        self.__timestamp = piw.tsd_time()
        self.__selected=None
        self.__update()

    def rpc_displayname(self,arg):
        a = self.get_description()
        return a + ' samples'

    def rpc_setselected(self,arg):
        (path,selected) = logic.parse_clause(arg)
        if len(path) == 1:
             self.__selected=selected
  
    def rpc_activated(self,arg):
        (path,selected) = logic.parse_clause(arg)
        if len(path) == 1:
            self.__setcurrent(selected)
        return logic.render_term(('',''))

    def __update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def __join(self,file,bank,preset):
        file = os.path.basename(file)
        return '|'.join((file,str(bank),str(preset)))

    def __split(self,cookie):
        if not cookie:
            return None

        f,b,p = cookie.split('|',2)
        q = self.__qualify(f)
        if q:
            try:
                return q,int(b),int(p)
            except:
                pass

        return None

    def __loadcurrent(self,delegate,v):
        desc = None
        cookie = ''

        if v.is_string():
            cookie = v.as_string()
            if cookie:
                desc = self.__split(cookie)
                if not desc:
                    delegate.add_error('Soundfont file %s not found' % cookie.split('|')[0])
                    cooke = ''

        self.agent.setup(desc)
        self.set_value(cookie)
        self.__update()
            

    def __setcurrent(self,cookie):
        desc = self.__split(cookie)

        if desc:
            self.agent.setup(desc)
            self.set_value(cookie)
            self.__update()
            return True

        return False

    def __ideals(self,*cookies):
        return '[%s]' % ','.join([self.__ideal(c) for c in cookies])

    def __ideal(self,cookie):
        return 'ideal([~server,sample],"%s")' % cookie

    def rpc_fideal(self,arg):
        (path,cookie) = logic.parse_clause(arg)
        if self.__split(cookie):
            i = self.__ideal(cookie)
            return i
        return async.failure('invalid cookie')

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)
        if a == ('current',) and o is None:
            c = self.get_value()
            if c:
                return self.__ideals(self.get_value())
        return self.__ideals()

    def rpc_dinfo(self,a):
        c=self.get_value()
        l=[]
        if c:
            l.append(('dinfo_id',c))
            l.append(('Current sample',c.split('|')[0]))
            return logic.render_term(T('keyval',tuple(l) ))
        else:
            n=self.get_description()
            l.append(('dinfo_id',n))

            return logic.render_term(T('keyval',tuple(l) ))
 
    def rpc_current(self,arg):
        c = self.get_value()

        if not c:
            return '[]'

        f,b,p = c.split('|',2)

        return '[["%s",["%s"]]]' % (c,f)

    def rpc_enumerate(self,a):
        path = logic.parse_clause(a)
        self.__scan()

        if len(path) == 0:
            return logic.render_term((0, len(self.__f2p)))

        if len(path) == 1:
            p = self.__scanpresets(path[0])
            return logic.render_term((len(p), 0))

        return logic.render_term((0,0))

    def rpc_cinfo(self,a):
        (path,idx) = logic.parse_clause(a)
        self.__scan()

        if len(path) == 0:
            return render_list(self.__files,idx,lambda i,t: logic.render_term((t)))

        return logic.render_term(())

    def rpc_finfo(self,a):
        (path,idx) = logic.parse_clause(a)

        if len(path) == 1:
            self.__scan()
            presets = self.__scanpresets(path[0])
            #return render_list(presets,idx,lambda i,t: logic.render_term((t[0],t[1],t[2],'autoload')))
            return render_list(presets,idx,lambda i,t: logic.render_term((t[0],t[1],t[2])))

        return logic.render_term(())

    def __scan1(self,pat,f=lambda x:x):
        g = lambda path: glob.glob(os.path.join(path,pat))
        paths = g(self.reldir) + g(self.userdir)
        b = lambda path: os.path.splitext(os.path.basename(path))[0]
        return map(b,paths), map(f,paths)

    def __scan(self):
        files,paths = self.__scan1('*.[sS][fF]2')
        self.__f2p = dict(zip(files,paths))
        self.__files = self.__f2p.keys()
        names,cookies = self.__scan1('*.name', lambda p: open(p).read().strip())
        self.__n2c = dict(zip(names,cookies))
        self.__c2n = dict(zip(cookies,names))

    def __qualify(self,name):
        self.__scan()
        return self.__f2p.get(name)

    def __scanpresets(self,file):
        path = self.__f2p.get(file)
        if not path:
            return []

        ret = []
        for n,p,b in sf2.sf_info(path):
            cookie = self.__join(file,b,p)
            ret.append((cookie,n,self.__c2n.get(cookie) or 'None'))
        return ret

    def choose_cookie(self,c):
        if self.__setcurrent(c):
            return action.nosync_return()
        return async.failure('choose_cookie failed %s' % c) 

    def resolve_name(self,n):
        if n=='selection':
            if self.__selected:
                return self.__ideals(self.__selected)
        
        n = n.replace(' ','_')
        self.__scan()
        c = self.__n2c.get(n)
        if c:
            return self.__ideals(c)
        return self.__ideals()

    def current(self):
        return self.__split(self.get_value())

    def first(self):
        self.__scan()
        p = self.__scanpresets(self.__files[0])
        if p:
            self.__setcurrent(p[0][0])
            return action.nosync_return()
        return async.failure('first failed')

    def next(self):
        self.__scan()

        c = self.get_value()
        if not c:
            return action.nosync_return()
        f,b,p = c.split('|',2)

        if self.__files: 
            if f in self.__files:
                all=[]
                for ps in self.__scanpresets(f):
                    all.append(ps[0])

                i=all.index(c)+1
                if i<len(all):
                    self.__setcurrent(all[i])
                    return action.nosync_return()
                else:
                    findex=(self.__files.index(f))+1
                    if findex>=len(self.__files):
                        findex=0
                    f=self.__files[findex]
                    for ps in self.__scanpresets(f):
                        self.__setcurrent(ps[0])
                        return action.nosync_return()
            else:
                ps=self.__scanpresets(self.__files[0])
                self.__setcurrent(ps[0])   
                return action.nosync_return()

        return async.failure('no files')

    def unname(self,cookie):
        self.__scan()
        n = self.__c2n.get(cookie)

        if n is None:
            return action.nosync_return()

        n = '%s.name' % n.replace(' ','_')
        path = os.path.join(self.userdir,n)
        try: os.unlink(path)
        except: pass

        self.__scan()
        self.__update()
        return action.nosync_return()

    def rename(self,cookie,name):
        n = '%s.name' % name.replace(' ','_')
        path = os.path.join(self.userdir,n)
        try: os.unlink(path)
        except: pass
        f = open(path,'w')
        f.write(cookie)
        f.close()
        self.__update()
        return action.nosync_return()

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self,signature=version,names='sampler oscillator',container=8,ordinal=ordinal)
        self.domain = piw.clockdomain_ctl()

        self[1] = atom.Atom()
        self[1][1] = bundles.Output(1,True,names="left audio output")
        self[1][10] = bundles.Output(2,True,names="right audio output")
        self[1][2] = bundles.Output(3,False,names="activation output")
        self[1][3] = bundles.Output(1,False,names="envelope output")
        self[1][4] = bundles.Output(2,False,names="delay output")
        self[1][5] = bundles.Output(3,False,names="attack output")
        self[1][6] = bundles.Output(4,False,names="hold output")
        self[1][7] = bundles.Output(5,False,names="decay output")
        self[1][8] = bundles.Output(6,False,names="sustain output")
        self[1][9] = bundles.Output(7,False,names="release output")

        self.audio_output = bundles.Splitter(self.domain, self[1][1], self[1][2], self[1][10])
        self.envelope_output = bundles.Splitter(self.domain, self[1][3], self[1][4], self[1][5], self[1][6], self[1][7], self[1][8], self[1][9])

        self.synth_player = sampler2_native.player(self.audio_output.cookie(),self.domain)
        self.synth_loader = sampler2_native.loader(self.synth_player,self.envelope_output.cookie(),self.domain)
        self.vdetector = piw.velocitydetect(self.synth_loader.cookie(),4,3)

        self[4] = atom.Atom(domain=domain.BoundedInt(1,1000),init=4,names="velocity sample",policy=atom.default_policy(self.__set_samples))
        self[5] = atom.Atom(domain=domain.BoundedFloat(0.1,10),init=4,names="velocity curve",policy=atom.default_policy(self.__set_curve))
        self[7] = atom.Atom(domain=domain.BoundedFloat(0.1,10),init=4,names="velocity scale",policy=atom.default_policy(self.__set_scale))

        self.loader_input = bundles.VectorInput(self.vdetector.cookie(), self.domain, signals=(1,4),threshold=5)
        self.player_input = bundles.VectorInput(self.synth_player.cookie(), self.domain, signals=(1,2,3),threshold=5)

        self[2] = atom.Atom()
        self[2][1]=atom.Atom(domain=domain.BoundedFloat(0,96000,rest=440), names='frequency input',ordinal=1,policy=self.player_input.merge_policy(1,False))
        self[2][2]=atom.Atom(domain=domain.BoundedFloat(-1200,1200), names='detune input',policy=self.player_input.merge_policy(2,False))
        self[2][3]=atom.Atom(domain=domain.BoundedFloat(0,1), names='activation input',policy=self.player_input.vector_policy(3,False))

        self[2][4]=atom.Atom(domain=domain.BoundedFloat(0,1), names='pressure input',policy=self.loader_input.vector_policy(4,False))
        self[2][5]=atom.Atom(domain=domain.BoundedFloat(-72,72), names='transpose', policy=atom.default_policy(self.__settranspose))
        self[2][6]=atom.Atom(domain=domain.BoundedFloat(0,96000,rest=440), names='frequency input',ordinal=2,policy=self.loader_input.merge_policy(1,False))

        self.__transpose = 0
        self[3] = Sample(self)
        self[6] = atom.Atom(domain=domain.Bool(), init=True, names='fade enable', protocols='input explicit', policy=atom.default_policy(self.__set_fade))

        self.add_verb2(1,'first([],None)',self.__first)
        self.add_verb2(2,'next([],None)',self.__next)
        self.add_verb2(3,'name([],None,role(None,[ideal([~server,sample]),singular]),role(to,[abstract]))',self.__name)
        self.add_verb2(4,'choose([],None,role(None,[ideal([~server,sample]),singular]))',self.__choose)
        self.add_verb2(5,'name([un],None,role(None,[ideal([~server,sample]),singular]))',self.__unname)

        self.synth_player.set_fade(True)

    def rpc_resolve_ideal(self,arg):
        (type,arg) = action.unmarshal(arg)

        if type=='sample':
            return self[3].resolve_name(' '.join(arg))

        return action.marshal(())

    def __set_fade(self,fade):
        self.synth_player.set_fade(fade)
        return True

    def __first(self,subj):
        return self[3].first()

    def __next(self,subj):
        return self[3].next()

    def __choose(self,subj,arg):
        (type,thing) = action.crack_ideal(action.arg_objects(arg)[0])
        return self[3].choose_cookie(thing)

    def __name(self,subj,thing,name):
        thing = action.crack_ideal(action.arg_objects(thing)[0])[1]
        name = action.abstract_string(name)
        return self[3].rename(thing,name)

    def __unname(self,subj,thing):
        thing = action.crack_ideal(action.arg_objects(thing)[0])[1]
        return self[3].unname(thing)

    def __settranspose(self,t):
        self.__transpose = t
        c = self[3].current()
        self.setup(c)
        return True

    def setup(self,desc):
        self.synth_loader.load(piw.preset())
        if desc:
            (filename,bank,preset) = desc
            preset = sf2.load_soundfont(filename, bank, preset, self.__transpose)
            self.synth_loader.load(preset)

    def __set_samples(self,x):
        self.vdetector.set_samples(x)
        return True

    def __set_curve(self,x):
        self.vdetector.set_curve(x)
        return True

    def __set_scale(self,x):
        self.vdetector.set_scale(x)
        return True

class Upgrader(upgrade.Upgrader):
    def upgrade_5_0_to_6_0(self,tools,address):
        root = tools.root(address)
        root.ensure_node(6,255,1).set_string('bool([])')
        root.ensure_node(6,255,8).set_string('fade enable')
        root.ensure_node(6,255,3)
        root.ensure_node(6,254).set_data(piw.makebool(False,0))
        return True

    def upgrade_4_0_to_5_0(self,tools,address):
        root = tools.root(address)
        root.ensure_node(8).erase_children()
        return True

    def upgrade_3_0_to_4_0(self,tools,address):
        root = tools.root(address)
        root.get_node(1,1).rename(names='audio output',adjectives='left')
        return True

    def upgrade_2_0_to_3_0(self,tools,address):
        root = tools.root(address)
        root.ensure_node(2,1,255,const.meta_ordinal).set_data(piw.makelong(1,0))

        return True

agent.main(Agent,Upgrader)

