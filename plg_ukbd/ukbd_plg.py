
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
from pi import atom, utils, bundles, domain, agent, paths, policy, action, logic, upgrade
from lib_micro import ezload
from . import micro_manager_version as version,ukbd_native

class VirtualKey(atom.Atom):
    def __init__(self):
        atom.Atom.__init__(self,names='key',protocols='virtual')
        self.choices=[]

    def __key(self,*keys):
        x = ','.join(['cmp([dsc(~(parent)"#1","%(k)d"),dsc(~(parent)"#2","%(k)d"),dsc(~(parent)"#3","%(k)d"),dsc(~(parent)"#4","%(k)d"),dsc(~(parent)"#7","%(k)d")])' % dict(k=k) for k in keys])
        return '[%s]' % x

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)
        print 'resolving virtual',(a,o)
        if not a and o is None: return self.__key(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17)
        if a==('standard',) and o is None: return self.__key(13,14,15,16,9,10,11,12,5,6,7,8,1,2,3,4)
        if a==('chosen',) and o is None: return self.__key(*self.choices)
        if a or o is None: return '[]'
        o=int(o)
        if o<1 or o>17: return '[]'
        return self.__key(o)

class Keyboard(agent.Agent):
    subsys_relation = ('create','by')

    def __init__(self,usbname,ordinal,dom,remove):
        agent.Agent.__init__(self,names='keyboard micro',ordinal=ordinal,subsystem='kbd',volatile=True,signature=version, container=102,protocols='is_subsys')

        self.usbname=usbname
        self.remover=remove

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))

        self[2] = bundles.Output(2,False, names='pressure output')
        self[3] = bundles.Output(3,False, names='roll output')
        self[4] = bundles.Output(4,False, names='yaw output')
        self[17] = bundles.Output(5,False,names='key output')

        self[5] = bundles.Output(1,False, names='strip position output')
        self[6] = bundles.Output(2,False, names='breath output')
        
        self.led_backend = piw.functor_backend(1,True)
        self.status_mixer = piw.statusmixer(self.led_backend.cookie())
        self.led_input = bundles.VectorInput(self.status_mixer.cookie(),self.domain,signals=(1,))
        self[7] = atom.Atom(names='light input',protocols='revconnect',policy=self.led_input.vector_policy(1,False,False,auto_slot=True),domain=domain.Aniso())

        self.koutput = bundles.Splitter(self.domain,self[2],self[3],self[4],self[17])
        self.kpoly = piw.polyctl(10,self.koutput.cookie(),False,5)
        self.aoutput = bundles.Splitter(self.domain,self[5],self[6])

        self.kclone=piw.sclone()
        self.kclone.set_filtered_output(1,self.kpoly.cookie(),piw.first_filter(1))
        self.kclone.set_filtered_output(2,self.aoutput.cookie(),piw.first_filter(2))
        self.keyboard=ukbd_native.bundle(usbname,self.kclone.cookie(),utils.notify(self.dead),utils.changify(self.signal))

        self[100] = VirtualKey()

        self[251] = atom.Atom(domain=domain.BoundedFloat(0,1), init=self.keyboard.get_threshold1(), protocols='input output', names='soft threshold', policy=atom.default_policy(self.keyboard.set_threshold1))
        self[249] = atom.Atom(domain=domain.BoundedFloat(0,1), init=self.keyboard.get_threshold2(), protocols='input output', names='hard threshold', policy=atom.default_policy(self.keyboard.set_threshold2))

        f=self.keyboard.led_functor()
        self.led_backend.set_functor(piw.pathnull(0),f)

        self[9] = atom.Atom(names='controller output',domain=domain.Aniso(),init=self.__controllerinit())

    def __controllerinit(self):
        return utils.makedict({'columnlen':self.keyboard.get_columnlen(),'columnoffset':self.keyboard.get_columnoffset(),'courselen':self.keyboard.get_courselen(),'courseoffset':self.keyboard.get_courseoffset()},0)

    def close_server(self):
        atom.Atom.close_server(self)
        if self.keyboard:
            self.keyboard.close()
            self.keyboard = None

    def name(self):
        return self.keyboard.name()

    def dead(self):
        self.close_server()
        self.remover()

    def signal(self,d):
        pass

class KeyboardAgent(agent.Agent):
    def __init__(self,address, ordinal):
        agent.Agent.__init__(self,signature=version, names='micro manager',protocols='has_subsys',ordinal=ordinal)
        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*', 0L))

        self.load_enumerator = picross.enumerator(ezload.vendor,ezload.product,picross.make_string_functor(self.download_keyboard))
        self.enumerator = picross.enumerator(0xbeca,0x0101,picross.make_string_functor(utils.make_locked_callable(self.add_keyboard)))

    def server_opened(self):
        agent.Agent.server_opened(self)
        self.enumerator.start()
        self.load_enumerator.start()

    def server_closed(self):
        self.load_enumerator.stop()
        self.enumerator.stop()
        agent.Agent.server_closed(self)

    def download_keyboard(self,usbname):
        firmware = ezload.firmware(ezload.vendor,ezload.product)
        if firmware:
            print 'loading firmware'
            ezload.download(usbname,firmware)

    def next_keyboard(self):
        i=0
        for ss in self.iter_subsystem():
            i=max(i,int(ss))
        return i+1

    def add_keyboard(self,usbname):
        i=self.next_keyboard()
        k=Keyboard(usbname,i,self.domain,lambda: self.del_keyboard(i))
        self.add_subsystem(str(i),k)
        print 'added keyboard',i,k.name()

    def del_keyboard(self,i):
        print 'removed keyboard',i
        self.remove_subsystem(str(i))


agent.main(KeyboardAgent)
