
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

import os
import picross
import piw

from pi import atom,bundles,domain,agent,logic,utils,node,action,async,upgrade
from . import midi_input_version as version,midi_native

class VirtualKey(atom.Atom):
    def __init__(self):
        atom.Atom.__init__(self,names='key',protocols='virtual')
        self.choices=[]

    def __key(self,*keys):
        x = ','.join(['cmp([dsc(~(parent)"#1","%(k)d")])' % dict(k=k) for k in keys])
        return '[%s]' % x

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)
        print 'resolving virtual',arg,(a,o)
        if not a and o is None: return self.__key(*range(0,128))
        if a==('chosen',) and o is None: return self.__key(*self.choices)
        if a or o is None: return self.__key()
        o=int(o)
        if o<0 or o>127: return self.__key()
        return self.__key(o)


class VirtualCC(atom.Atom):
    clist = (
        ('bank select coarse', 0), ('modulation wheel coarse', 1), ('breath controller coarse', 2), ('foot pedal coarse', 4),
        ('portamento time coarse', 5), ('data entry coarse', 6), ('volume coarse', 7), ('balance coarse', 8),
        ('pan position coarse', 10), ('expression coarse', 11), ('effect control 1 coarse', 12), ('effect control 2 coarse', 13),
        ('general purpose slider 1', 16), ('general purpose slider 2', 17), ('general purpose slider 3', 18), ('general purpose slider 4', 19),
        ('bank select fine', 32), ('modulation wheel fine', 33), ('breath controller fine', 34), ('foot pedal fine', 36),
        ('portamento time fine', 37), ('data entry fine', 38), ('volume fine', 39), ('balance fine', 40),
        ('pan position fine', 42), ('expression fine', 43), ('effect control 1 fine', 44), ('effect control 2 fine', 45),
        ('hold pedal', 64), ('portamento', 65), ('sustenuto pedal', 66), ('soft pedal', 67),
        ('legato pedal', 68), ('hold 2 pedal', 69), ('sound variation', 70), ('sound timbre', 71),
        ('sound release time', 72), ('sound attack time', 73), ('sound brightness', 74), ('sound control 6', 75),
        ('sound control 7', 76), ('sound control 8', 77), ('sound control 9', 78), ('sound control 10', 79),
        ('general purpose button 1', 80), ('general purpose button 2', 81), ('general purpose button 3', 82), ('general purpose button 4', 83),
        ('effects level', 91), ('tremulo level', 92), ('chorus level', 93), ('celeste level', 94),
        ('phaser level', 95), ('data button increment', 96), ('data button decrement', 97), ('non-registered parameter fine', 98),
        ('non-registered parameter coarse', 99), ('registered parameter fine', 100), ('registered parameter coarse', 101), ('all sound off', 120),
        ('all controllers off', 121), ('local keyboard', 122), ('all notes off', 123), ('omni mode off', 124),
        ('omni mode on', 125), ('mono operation', 126), ('poly operation', 127))

    cdict = dict(clist)

    def __init__(self):
        atom.Atom.__init__(self,names='continuous controller',protocols='virtual browse')
        self.__selected=None

    def rpc_setselected(self,arg):
        print 'VirtualCC:setselected',arg    
    
    def rpc_activated(self,arg):
        print 'VirtualCC:activated',arg    
        return logic.render_term(('',''))

    def rpc_current(self,arg):
        return '[]'

    def __key(self,*keys):
        x = ','.join(['cmp([dsc(~(parent)"#2","%(k)d")])' % dict(k=k) for k in keys])
        return '[%s]' % x

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)
        a = (' '.join(a)).lower()
        print 'midi cc resolving',a,o
        if a in self.cdict: return self.__key(self.cdict[a])
        a2 = a+' coarse'
        if a2 in self.cdict: return self.__key(self.cdict[a2])
        if not a and o is None: return self.__key(*range(0,128))
        if a or o is None: return self.__key()
        o=int(o)
        if o<0 or o>127: return self.__key()
        print 'resolved to',self.__key(o)
        return self.__key(o)

    def rpc_enumerate(self,a):
        return logic.render_term((len(self.clist),0))

    def rpc_cinfo(self,a):
        return '[]'

    def rpc_finfo(self,a):
        (path,idx) = logic.parse_clause(a)
        map = tuple([ (str(s),'cc %d: %s' % (s,n),None) for (n,s) in self.clist[idx:] ])
        return logic.render_term(map)

    def rpc_fideal(self,arg):
        try:
            (path,cookie) = logic.parse_clause(arg)
            cookie=int(cookie)
        except:
            utils.log_exception()
            return async.failure('invalid cookie')
        for name,val in self.clist:
            if cookie==val:
                return 'cmp([dsc(~(parent)"#2",%d)])' % val
        return async.failure('invalid cookie')

class MidiDelegate(midi_native.midi_input):
    def __init__(self,cookie,midi_cookie,notify):
        midi_native.midi_input.__init__(self,cookie,midi_cookie)
        self.sources = []
        self.__notify = notify

    def source_added(self,id,name):
        xid = '%x'%id
        for i,(u,n) in enumerate(self.sources):
            if u==xid:
                print 'midi source changed',xid,name
                self.sources[i] = (xid,name)
                self.__notify()
                return
        print 'midi source added',xid,name
        self.sources.append((xid,name))
        self.__notify()

    def source_removed(self,id):
        xid = '%x'%id
        for i,(u,n) in enumerate(self.sources):
            if u==xid:
                print 'midi source removed',xid,n
                del self.sources[i]
                self.__notify()
                return

class MidiPort(atom.Atom):
    def __init__(self,cookie,midi_cookie):
        self.__timestamp = piw.tsd_time()
        self.__midi = MidiDelegate(cookie,midi_cookie,self.__sinks_changed)

        atom.Atom.__init__(self,domain=domain.String(),names='midi port',policy=atom.default_policy(self.setport),protocols='virtual browse')
        self.__midi.setport(0)
        self.__midi.set_destination('')
        self.__selected=None
        self.__update()
        self.__index = 1

    def set_index(self,index):
        self.__index = index
        if self.open():
            self.__midi.set_destination('Eigenlabs %d' % self.__index)

    def server_opened(self):
        atom.Atom.server_opened(self)
        self.__midi.set_destination('Eigenlabs %d' % self.__index)
        self.__midi.run()
        self.setport(self.get_value())

    def close_server(self):
        atom.Atom.close_server(self)
        self.__midi.set_destination('')
        self.__midi.setport(0)
        self.__midi.stop()

    def __update(self):
        if not self.get_value() and len(self.__midi.sources):
            port = self.__midi.sources[0][0]
            self.__midi.setport(int(port,16))

        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def setport(self,port):
        if self.open():
            print 'set port to',port
            if port:
                self.__midi.setport(int(port,16))
            else:
                if len(self.__midi.sources):
                    self.__midi.setport(int(self.__midi.sources[0][0],16))
        self.set_value(port)
        self.__update()

    def __sinks_changed(self):
        self.setport(self.get_value())

    def rpc_displayname(self,arg):
        return 'MIDI input ports'

    def rpc_setselected(self,arg):
        (path,selected)=logic.parse_clause(arg)
        print 'MidiPort:setselected',selected    
        self.__selected=selected
    
    def rpc_activated(self,arg):
        (path,selected)=logic.parse_clause(arg)
        print 'MidiPort:activated',selected    
        port=selected
        self.setport(port)
        return logic.render_term(('',''))
    
    def clear_trim(self):
        self.__midi.clear_trim()

    def set_trim(self,cc,min,max,inv):
        self.__midi.set_trim(cc,min,max,inv)

    def current(self,cc):
        return self.__midi.current(cc)

    def resolve_name(self,name):
        if name=='selection':
#            o=self.__selected
            return self.__ideal(self.__selected)
        else:
            try:
                o = int(name)
            except:
                return '[]'

        if o>0 and o<len(self.__midi.sources)+1:
            return self.__ideal(self.__midi.sources[o-1][0])

        return '[]'

    def __ideal(self,uid):
        return '[ideal([~server,midiport],%s)]' % logic.render_term(uid)

    def rpc_fideal(self,arg):
        (path,cookie) = logic.parse_clause(arg)
        for id,n in self.__midi.sources:
            if id==cookie:
                return 'ideal([~server,midiport],%s)' % logic.render_term(cookie)
        return async.failure('invalid cookie')

    def rpc_current(self,arg):
        current = self.__midi.getport()
        if current==0:
            return '[]'
        return '[["%x",[]]]' % current

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)
        if a or not o:
            return '[]'
        return self.resolve_name(o)

    def rpc_enumerate(self,a):
        return logic.render_term((len(self.__midi.sources),0))

    def rpc_cinfo(self,a):
        return '[]'

    def rpc_finfo(self,a):
        (dlist,cnum) = logic.parse_clause(a)
        map = tuple([(uid,dsc,None) for (uid,dsc) in self.__midi.sources[cnum:]])
        return logic.render_term(map)

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self,names='midi input',signature=version,container=6,ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))
        self.set_private(node.Server(value=piw.makestring('[]',0), change=self.__settrim))

        self[1] = bundles.Output(1,False,names='key output')
        self[2] = bundles.Output(2,False,names='continuous controller output')
        self[8] = bundles.Output(3,False,names='program change output')

        self.output = bundles.Splitter(self.domain,self[1],self[2],self[8])

        self[6] = bundles.Output(1,False,names='midi output')
        self[7] = bundles.Output(2,False,names='midi clock output')
        self.midi_output = bundles.Splitter(self.domain,self[6],self[7])

        self[3] = VirtualKey()
        self[4] = VirtualCC()
        self[5] = MidiPort(self.output.cookie(),self.midi_output.cookie())

        self.add_verb2(2,'choose([],None,role(None,[ideal([~server,midiport]),singular]))',self.__chooseport)
        self.add_verb2(3,'invert([],None,role(None,[cmpdsc([~(s)"#2"])]))', self.__invert);
        self.add_verb2(4,'minimise([],None,role(None,[cmpdsc([~(s)"#2"])]),option(to,[numeric]))', self.__setmin);
        self.add_verb2(5,'maximise([],None,role(None,[cmpdsc([~(s)"#2"])]),option(to,[numeric]))', self.__setmax);

        self.set_ordinal(ordinal)

    def property_change(self,key,value,delegate):
        if key == 'ordinal':
            self[5].set_index(self.get_property_long('ordinal',1))

    def __settrim(self,val):
        if val.is_string():
            trim = logic.parse_clause(val.as_string())
            self[5].clear_trim()
            for (cc,min,max,inv) in trim:
                self[5].set_trim(cc,min,max,inv)
            print 'trim:',trim
            self.get_private().set_data(val)

    def get_trim(self,cc):
        trim = logic.parse_clause(self.get_private().get_data().as_string())
        for (tcc,min,max,inv) in trim:
            if tcc==cc:
                return list((tcc,min,max,inv))
        return [cc,0,127,False]

    def set_trim(self,cc,min,max,inv):
        trim = list(logic.parse_clause(self.get_private().get_data().as_string()))
        done = False

        for (i,(tcc,tmin,tmax,tinv)) in enumerate(trim):
            if tcc==cc:
                trim[i] = (cc,min,max,inv)
                done = True

        if not done:
            trim.append((cc,min,max,inv))

        self[5].set_trim(cc,min,max,inv)
        trim = logic.render_term(tuple(trim))
        self.get_private().set_data(piw.makestring(trim,0))

    def __invert(self,subj,arg):
        cc = int(arg[0].args[0][0].args[1])
        print 'invert controller',cc
        trim = self.get_trim(cc)
        trim[3] = not trim[3]
        self.set_trim(*trim)

    def __setmin(self,subj,arg,val):
        cc = int(arg[0].args[0][0].args[1])

        if val is None:
            val=self[5].current(cc) 
        else:
            val=int(action.abstract_string(val))

        print 'set controller minimum',cc,val
        trim = self.get_trim(cc)
        trim[1] = val

        if trim[1]<=trim[2]:
            trim[3]=False
        else:
            trim[3]=True
            a=trim[1]
            trim[1]=trim[2]
            trim[2]=a

        self.set_trim(*trim)

    def __setmax(self,subj,arg,val):
        cc = int(arg[0].args[0][0].args[1])

        if val is None:
            val=self[5].current(cc) 
        else:
            val=int(action.abstract_string(val))

        print 'set controller maximum',cc,val
        trim = self.get_trim(cc)
        trim[2] = val

        if trim[1]<=trim[2]:
            trim[3]=False
        else:
            trim[3]=True
            a=trim[1]
            trim[1]=trim[2]
            trim[2]=a

        self.set_trim(*trim)

    def rpc_resolve_ideal(self,arg):
        (type,arg) = action.unmarshal(arg)
        print 'resolving',arg

        if type=='midiport':
            return self[5].resolve_name(' '.join(arg))

        return action.marshal(())

    def __chooseport(self,subj,arg):
        print 'choose port',arg
        print action.arg_objects(arg)[0]
        (type,thing) = action.crack_ideal(action.arg_objects(arg)[0])
        print type,thing
        self[5].setport(thing)


agent.main(Agent,gui=True)
