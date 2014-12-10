
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

from pi import agent,bundles,atom,action,domain,paths,upgrade,const,policy,node,resource,logic,utils
from plg_finger import fingerer_version as version

import piw
import finger_native
import ConfigParser
import os
import shutil

def simple_whistle():
    return [
            [
                [[1.0], [('1', '1'), ('1', '2'), ('1', '3'), ('1', '5'), ('1', '6'), ('1', '7')]], 
                [[2.0], [('1', '1'), ('1', '2'), ('1', '3'), ('1', '5'), ('1', '6')]], 
                [[3.0], [('1', '1'), ('1', '2'), ('1', '3'), ('1', '5')]], 
                [[4.0], [('1', '1'), ('1', '2'), ('1', '3')]], 
                [[5.0], [('1', '1'), ('1', '2')]], 
                [[6.0], [('1', '1')]], 
                [[7.0], [('0', '0')]]], 
            [
                [[-1.0], [('1', '4')]]], 
            [
                [[7.0], [('1', '8')]]], 
            [
                [[''], ['']]], 
            None
    ]


user_cat = '#User Fingerings'
factory_cat = '#Factory Fingerings'
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


class FingeringError(Exception):
    pass


class Fingering(atom.Atom):
    def __init__(self, callback):
        self.__callback = callback

        self.__user_file_name = resource.user_resource_file('Fingerer','User Fingerings.txt',version='') 
        self.__factory_file_name = os.path.join(os.path.dirname(__file__),'Factory Fingerings.txt')
        self.__user_seed_file = resource.find_release_resource('fingerer','User Fingerings.txt')
        self.__timestamp = piw.tsd_time()
        self.__timer = piw.thing()
        self.__timer.set_slow_timer_handler(utils.notify(self.__timer_callback))

        if not resource.os_path_exists(self.__user_file_name) and resource.os_path_exists(self.__user_seed_file):
            resource.shutil_copyfile(self.__user_seed_file,self.__user_file_name)

        self.__user_file_time = 0
        self.__factory_fingerings = {}
        self.__user_fingerings = {}

        self.__load_factory()
        self.__load_user()

        fn = set(self.__factory_fingerings.keys())
        fn.update(set(self.__user_fingerings.keys()))

        atom.Atom.__init__(self, names='fingering', domain=domain.String(), init='simple whistle', policy=atom.default_policy(self.__change_fingering), protocols='browse')

    def default_fingering(self):
        return self.__get_fingering("simple whistle")

    def server_opened(self):
        atom.Atom.server_opened(self)
        self.__timer.close_thing()
        piw.tsd_thing(self.__timer)
        self.__timer.timer_slow(5000)

    def __timer_callback(self):
        if self.__load_user():
            self.update()

    def close_server(self):
        atom.Atom.close_server(self)
        self.__timer.close_thing()

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def rpc_enumerate(self,arg):
        path = logic.parse_clause(arg)
        if len(path)>0:
            if path[0]==factory_cat:
                return logic.render_term((len(self.__factory_fingerings),0))
            if path[0]==user_cat:
                return logic.render_term((len(self.__user_fingerings),0))

        return logic.render_term((0,2))

    def __cinfo0(self,path):
        if len(path)==0:
            r = []
            r.insert(0,(factory_cat,))
            r.insert(0,(user_cat,))
            return r

        return []

    def __finfo0(self,path):
        if len(path) != 1:
            return []

        if path[0]==factory_cat:
            return [(f,f,'') for f in self.__factory_fingerings]

        if path[0]==user_cat:
            return [(f,f,'') for f in self.__user_fingerings]

        return []

    def rpc_finfo(self,arg):
        (path,idx) = logic.parse_clause(arg)
        return render_list(self.__finfo0(path),idx,lambda i,t: logic.render_term((t[0],str(t[1]),str(t[2]))))

    def rpc_cinfo(self,arg):
        (path,idx) = logic.parse_clause(arg)
        return render_list(self.__cinfo0(path),idx,lambda i,t: logic.render_term((str(t[0]))))

    def rpc_displayname(self,arg):
        return 'fingerings'

    def rpc_fideal(self,arg):
        return async.failure('invalid cookie')

    def rpc_activated(self,arg):
        (path,selected)=logic.parse_clause(arg)
        self.__change_fingering(selected)

    def rpc_setselected(self,arg):
        (path,selected)=logic.parse_clause(arg)

    def rpc_current(self,arg):
        return logic.render_term(((self.get_value(),()),))

    def __get_fingering(self,fingering_name):
        fingering = self.__user_fingerings.get(fingering_name)

        if fingering:
            return fingering

        fingering = self.__factory_fingerings.get(fingering_name)

        if fingering:
            return fingering

        return None

    def __change_fingering(self,fingering_name):
        fingering = self.__get_fingering(fingering_name)

        if not fingering:
            piw.tsd_alert('BAD_FINGERING', 'Fingering Set Error', ('Trying to set a non existent fingering: %s' % fingering))
            return False

        if self.__callback(fingering):
            self.set_value(fingering_name)
            self.update()
            print 'fingering set to',fingering_name

        return False

    def __load_factory(self):
        self.__factory_fingerings['simple whistle'] = simple_whistle

        (f,e) = self.__read_fingering(self.__factory_file_name)

        if e:
            piw.tsd_alert('BAD_FINGERING', 'Fingering File Error', e)
        else:
            self.__factory_fingerings.update(f)

    def __load_user(self):
        if not resource.os_path_exists(self.__user_file_name):
            self.__user_fingerings = {}
            return False

        t = resource.os_path_getmtime(self.__user_file_name)

        if t <= self.__user_file_time:
            return False

        self.__user_fingerings = {}
        self.__user_file_time = t
        (f,e) = self.__read_fingering(self.__user_file_name)

        if e:
            piw.tsd_alert('BAD_FINGERING', 'Fingering File Error', e)
            return False

        self.__user_fingerings = f
        return True


    def __read_fingering(self,filename):
        config = ConfigParser.ConfigParser()
        fingerings_from_this_file = {}
        message = ''

        try:
            config.read(resource.WC(filename))

            for fingering_name in config.sections():
                try:
                    options = config.options(fingering_name)
                    fingering_name = fingering_name.strip()
                    fingerings_from_this_file[fingering_name] = [[],[],[],[],None]

                    for option in options:
                        line = config.get(fingering_name,option)
                        pattern = [[],[]]
                        keys, targets = line.split('*',2)
                        targets = targets.strip()

                        if keys.count ('open'):
                            fingerings_from_this_file[fingering_name][4]=True
                            pattern[0].append(float(targets.strip()))
                            pattern[1].append(('0', '0'))
                            i=0;
                        else:
                            for target in targets.split(' '):
                                pattern[0].append(float(target.strip()))

                            keys = keys.strip()
                            coords = keys.split(' ')

                            for coord in coords:
                                coord.strip()
                                course, key = coord.split(',',2)
                                pattern[1].append((course, key)) 

                            if option.count('finger'): i = 0
                            elif option.count('mod'): i = 1
                            elif option.count('add'): i = 2
                            elif option.count('poly'): 
                                i = 3
                                if len(pattern[1]) >1:
                                    raise FingeringError('More than one activation key defined for a polyphony note')

                        fingerings_from_this_file[fingering_name][i].append(pattern)

                except FingeringError,e:
                    raise FingeringError("In fingering: '%s', %s" % (fingering_name,e.args[0]))
                except:
                    raise FingeringError("In fingering: '%s', unknown error" % fingering_name)

                print 'added',fingering_name,'from',filename

            print 'read %d fingerings from %s' % (len(fingerings_from_this_file),filename)
            return (fingerings_from_this_file,None)
                    
        except FingeringError,e:
            message = e.args[0]
        except:
            message = 'unknown error'

        return (None,'In file %s: %s' % (filename,message))



class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))

        agent.Agent.__init__(self, signature=version, names='fingerer',protocols='bind set',container=(5,'fingerer',atom.VerbContainer(clock_domain=self.domain)),ordinal=ordinal)


        self.__maximum = 0.1
        self.__threshold = 0.0
        self.__breath_threshold = 0.001

        self[1] = atom.Atom(names='outputs')
        self[1][1] = bundles.Output(1,False,names='pressure output', protocols='')
        self[1][2] = bundles.Output(2,False,names='roll output', protocols='')
        self[1][3] = bundles.Output(3,False,names='yaw output', protocols='')
        self[1][4] = bundles.Output(4,False,names='key output', protocols='')
        self[1][5] = bundles.Output(5,False,names='modifier output', protocols='')

        self.output = bundles.Splitter(self.domain,*self[1].values())
        self.finger = finger_native.cfinger(self.domain,self.output.cookie())
        self.input = bundles.VectorInput(self.finger.cookie(),self.domain,signals=(1,2,3,4,5))

        self[5] = Fingering(self.__change_fingering)

        self.current_fingering = finger_native.fingering()

        self.__set_fingering(self[5].default_fingering(), self.current_fingering)
        self.current_fingering.set_range(self.__threshold,self.__maximum)
        self.current_fingering.set_breath_threshold(self.__breath_threshold)
        self.finger.set_fingering(self.current_fingering)

        self[4]=atom.Atom(names='inputs')
        self[4][1]=atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.input.merge_policy(1,False),names='pressure input',init=1.0)
        self[4][2]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.input.merge_policy(2,False),names='roll input')
        self[4][3]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.input.merge_policy(3,False),names='yaw input')
        self[4][4]=atom.Atom(domain=domain.Aniso(),policy=self.input.vector_policy(4,False), names='key input')
        self[4][5]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.input.local_policy(5,False),names='breath input',init=1.0)


        self[6]=atom.Atom(names='key parameter')
        self[6][1] = atom.Atom(domain=domain.BoundedFloat(0,1), names='maximum', policy=atom.default_policy(self.__change_maximum),init=self.__maximum)
        self[6][2] = atom.Atom(domain=domain.BoundedFloat(0,1), names='threshold', policy=atom.default_policy(self.__change_threshold),init=self.__threshold)

        self[7]=atom.Atom(names='breath parameter')
        self[7][1] = atom.Atom(domain=domain.BoundedFloat(0,1), names='activation threshold', policy=atom.default_policy(self.__change_breath_threshold),init=self.__breath_threshold)

    def __change_fingering(self,fingering):
        self.__set_fingering(fingering, self.current_fingering)
        self.finger.set_fingering(self.current_fingering)
        return True

    def __change_maximum(self,maximum):
        self.__maximum = maximum
        self.current_fingering.set_range(self.__threshold,self.__maximum)
        self.finger.set_fingering(self.current_fingering)
        self[6][1].set_value(self.__maximum)
        return False

    def __change_threshold(self,threshold):
        self.__threshold = threshold
        self.current_fingering.set_range(self.__threshold,self.__maximum)
        self.finger.set_fingering(self.current_fingering)
        self[6][2].set_value(self.__threshold)
        return False

    def __change_breath_threshold(self,breath_threshold):
        self.__breath_threshold = breath_threshold
        self.current_fingering.set_breath_threshold(self.__breath_threshold)
        self.finger.set_fingering(self.current_fingering)
        self[7][1].set_value(self.__breath_threshold)
        return False

    def __set_fingering(self, f, current_fingering):

        print 'setting fingering to',f

        current_fingering.clear_table()
        needed_polyphony = 1;

        pattern_number = 0
        for pattern in f[0]:
            current_fingering.add_fingering_pattern(pattern_number,pattern[0][0])
            for k in pattern[1]:
                course = int(k[0])
                key = int(k[1])
                current_fingering.add_key(pattern_number, course, key)
            pattern_number += 1

        pattern_number = 0
        for pattern in f[1]:
            current_fingering.add_modifier_pattern(pattern_number,pattern[0][0])
            for k in pattern[1]:
                course = int(k[0])
                key = int(k[1])
                current_fingering.add_modifier_key(pattern_number, course, key)
            pattern_number += 1

        pattern_number = 0
        for pattern in f[2]:
            current_fingering.add_addition_pattern(pattern_number,pattern[0][0])
            for k in pattern[1]:
                course = int(k[0])
                key = int(k[1])
                current_fingering.add_addition_key(pattern_number, course, key)
            pattern_number += 1

        pattern_number = 0
        for pattern in f[3]:
            needed_polyphony += 1
            current_fingering.add_polyphony_pattern(pattern_number,pattern[0][0])
            for k in pattern[1]:
                course = int(k[0])
                key = int(k[1])
                current_fingering.add_polyphony_key(pattern_number, course, key)
            pattern_number += 1

        if(f[4]):
            current_fingering.set_open()

        current_fingering.evaluate_table()
        current_fingering.set_poly(needed_polyphony)
        self.finger.set_fingering(self.current_fingering)
        return
  

class Upgrader(upgrade.Upgrader):
    pass

agent.main(Agent,Upgrader)
