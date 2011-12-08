
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

from pi import agent,atom,action,bundles,domain,errors,paths,policy,async,logic,const,node,upgrade,utils,container,timeout,paths,guid
from plg_simple import kgroup_version as version
from pi.logic.shortcuts import *
import piw
import picross

def removelist(l1,s):
    l2=[]
    for l in l1:
        if l not in s:
            l2.append(l)
    return l2


def calc_keynum(geo,row,col):
    if not geo or not geo.is_tuple():
        return None
    
    geolen = geo.as_tuplelen();
    if geolen <= 0:
        return None

    if 0 == col:
        return None

    # the key was entered in a sequential column-only format
    if 0 == row:
        if col > 0:
            return col
        else:
            return 0
    
    # resolve relative rows
    if row < 0:
        row = geolen + row + 1

    # only calculate the key number when the row exists
    if row < 1 or row > geolen:
        return None
    
    rowlen = geo.as_tuple_value(row-1).as_long()

    # resolve relative columns
    if col < 0:
        col = rowlen + col + 1

    # only calculate the key number when the column exists
    if rowlen < col:
        return None

    # the row and column are within existing bounds, iterate
    # through the geometry to calculate the key number
    if row > 0 and col > 0:
        keynum = 0
        for i in range(1,row):
            keynum += geo.as_tuple_value(i-1).as_long()
        keynum += col

        return keynum

    return None

    
class SlowTonicChange:
    def __init__(self,agent,tonic):
        self.__agent = agent
        self.__tonic = tonic

    def change(self,d):
        if d.as_norm() != 0:
            self.__agent[19].set_value(self.__tonic)


class SlowScaleChange:
    def __init__(self,agent,scale):
        self.__agent = agent
        self.__scale = scale 

    def change(self,d):
        if d.as_norm() != 0:
            self.__agent[20].set_value(self.__scale)


class Controller:
    def __init__(self,agent,cookie):
        self.__agent = agent
        self.__dict = piw.controllerdict(cookie)
        self.__state = node.Server(change=self.__change_controller)

    def changetonic(self,t):
        return self.__dict.changetonic(t)

    def changescale(self,s):
        return self.__dict.changescale(s)

    def state(self):
        return self.__state

    def __change_controller(self,d):
        if d.is_dict():
            for x in ('courseoffset','courselen'):
                v = d.as_dict_lookup(x)
                self.__dict.put_ctl(x,v)
            self.__state.set_data(self.__dict.get_ctl_dict().make_normal())
            self.__agent.controller_changed()

    def thing_changed(self,d,kk):
        self.__dict.put(kk,d)

    def set_course_semis(self,idx,val):
        self.ensure(idx)
        l = [i.as_float() for i in self.getlist('courseoffset')]
        l[idx-1] = float(val)
        self.setlist('courseoffset',[piw.makefloat(i,0) for i in l])

    def set_course_steps(self,idx,val):
        self.ensure(idx)
        l = [i.as_float() for i in self.getlist('courseoffset')]
        l[idx-1] = float(val+10000 if val else 0)
        self.setlist('courseoffset',[piw.makefloat(i,0) for i in l])

    def get_courses(self):
        o = [i.as_float() for i in self.getlist('courseoffset')]
        l = [i.as_long() for i in self.getlist('courselen')]
        s = min(len(o),len(l))
        c = []
        i = 0
        while i < s:
           c.append((l[i],o[i])) 
           i = i +1
        return tuple(c)

    def set_courses(self,courses):
        lengths = []
        offsets = []
        for l,o in courses:
            lengths.append(piw.makelong(l,0))
            offsets.append(piw.makefloat(o,0))
        self.setlist('courselen',lengths)
        self.setlist('courseoffset',offsets)

    def num_courses(self):
        l = self.getlist('courseoffset')
        return len(l) if l else 0

    def ensure(self,course):
        l = [i.as_float() for i in self.getlist('courseoffset')]
        if course>len(l):
            xl = course-len(l)
            l.extend([0]*xl)
            self.setlist('courseoffset',[piw.makefloat(i,0) for i in l])

        l = [i.as_long() for i in self.getlist('courselen')]
        if course>len(l):
            xl = course-len(l)
            l.extend([0]*xl)
            self.setlist('courselen',[piw.makelong(i,0) for i in l])

    def setlist(self,name,l):
        self.__dict.put_ctl(name, utils.maketuple(l,piw.tsd_time()))
        self.__state.set_data(self.__dict.get_ctl_dict())
        self.__agent.controller_changed()

    def settuple(self,name,t):
        self.__dict.put_ctl(name, t)
        self.__state.set_data(self.__dict.get_ctl_dict())
        self.__agent.controller_changed()

    def getlist(self,name):
        d = self.gettuple(name)
        if d is None or not d.is_tuple():
            return []
        return utils.tuple_items(d)

    def gettuple(self,name):
        d = self.__dict.get_ctl_value(name)
        if d is None or not d.is_tuple():
            return piw.tuplenull(piw.tsd_time())
        return d;
        

class VirtualKey(atom.Atom):
    def __init__(self,size,chosen):
        atom.Atom.__init__(self,names='k',protocols='virtual')
        self.__size=size
        self.__chosen=chosen

    def __key(self,*keys):
        x=','.join(['cmp([dsc(~(a)"#1","%(k)d"),dsc(~(a)"#2","%(k)d"),dsc(~(a)"#3","%(k)d"),dsc(~(a)"#4","%(k)d"),dsc(~(a)"#8","%(k)d")])' % dict(k=k) for k in keys])
        return '[%s]' % x

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)
        if not a and o is None: return self.__key(*range(1,1+self.__size()))
        if a==('chosen',) and o is None: return self.__key(*self.__chosen())
        if a or o is None: return '[]'
        o=int(o)
        if o<1 or o>self.__size(): return '[]'
        return self.__key(o)


class VirtualCourse(atom.Atom):
    def __init__(self,controller):
        self.__controller = controller
        atom.Atom.__init__(self,names='course',protocols='virtual')

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)
        if a or o is None: return '[]'
        o=int(o)
        if o<1 or o>self.__controller.num_courses(): return '[]'
        return '[ideal([~server,course],%d)]'%o


class Output(atom.Atom):
    def __init__(self,master,slot):
        self.__agent = master.agent
        self.__list = master
        self.__slot = slot
        self.__tee = None

        atom.Atom.__init__(self,domain=domain.Bool(),init=False,policy=atom.default_policy(self.enable),names='kgroup output',protocols='remove',ordinal=slot,container=const.verb_node)

        self.light_output = piw.clone(True)
        self.light_input = bundles.VectorInput(self.light_output.cookie(),self.__agent.domain,signals=(1,))

        self[1] = bundles.Output(1,False, names='activation output')
        self[2] = bundles.Output(2,False, names='pressure output')
        self[3] = bundles.Output(3,False, names='roll output')
        self[4] = bundles.Output(4,False, names='yaw output')
        self[22] = bundles.Output(5,False, names='key output')
        self[5] = bundles.Output(1,False, names='strip position output',ordinal=1)
        self[9] = bundles.Output(1,False, names='strip position output',ordinal=2)
        self[6] = bundles.Output(1,False, names='breath output')
        self[7] = bundles.Output(1,False, names='controller output', continuous=True)
        self[8] = atom.Atom(names='light input',protocols='revconnect',policy=self.light_input.vector_policy(1,False,clocked=False,auto_slot=True),domain=domain.Aniso())
        self[10] = bundles.Output(2,False, names='absolute strip output',ordinal=1)
        self[11] = bundles.Output(2,False, names='absolute strip output',ordinal=2)
        self[30] = bundles.Output(1,False,names='pedal output', ordinal=1, protocols='')
        self[31] = bundles.Output(1,False,names='pedal output', ordinal=2, protocols='')
        self[32] = bundles.Output(1,False,names='pedal output', ordinal=3, protocols='')
        self[33] = bundles.Output(1,False,names='pedal output', ordinal=4, protocols='')

        self[20] = VirtualKey(self.__agent.kgroup_size,self.__agent.key_choice)

        self[24] = atom.Atom(domain=domain.BoundedInt(-32767,32767), names='key row', init=None, policy=atom.default_policy(self.__change_key_row))
        self[25] = atom.Atom(domain=domain.BoundedInt(-32767,32767), names='key column', init=None, policy=atom.default_policy(self.__change_key_column))

        self.koutput = bundles.Splitter(self.__agent.domain,self[1],self[2],self[3],self[4],self[22])
        self.s1output = bundles.Splitter(self.__agent.domain,self[5],self[10])
        self.s2output = bundles.Splitter(self.__agent.domain,self[9],self[11])
        self.boutput = bundles.Splitter(self.__agent.domain,self[6])
        self.coutput = bundles.Splitter(self.__agent.domain,self[7])
        self.output_pedal1 = bundles.Splitter(self.__agent.domain,self[30])
        self.output_pedal2 = bundles.Splitter(self.__agent.domain,self[31])
        self.output_pedal3 = bundles.Splitter(self.__agent.domain,self[32])
        self.output_pedal4 = bundles.Splitter(self.__agent.domain,self[33])

        self.kpolyctl = piw.polyctl(10,self.koutput.cookie(),False,5)

    def __change_key_row(self,val):
        self[24].set_value(val)
        self.update_status_index()
        return False

    def __change_key_column(self,val):
        self[25].set_value(val)
        self.update_status_index()
        return False

    def __setstate(self,data):
        self.set_value(data.as_norm()!=0.0)

    def enable(self,val):
        self.__agent.mode_selector.select(self.__slot,val)
        return True

    def plumb(self):
        self.unplumb()

        tee = piw.changelist_nb()
        self.__tee = tee

        n = self.__slot

        piw.changelist_connect_nb(tee,self.__agent.kclone.gate(n))
        piw.changelist_connect_nb(tee,self.__agent.s1clone.gate(n))
        piw.changelist_connect_nb(tee,self.__agent.s2clone.gate(n))
        piw.changelist_connect_nb(tee,self.__agent.bclone.gate(n))
        piw.changelist_connect_nb(tee,self.__agent.pedal1_clone.gate(n))
        piw.changelist_connect_nb(tee,self.__agent.pedal2_clone.gate(n))
        piw.changelist_connect_nb(tee,self.__agent.pedal3_clone.gate(n))
        piw.changelist_connect_nb(tee,self.__agent.pedal4_clone.gate(n))

        self.__agent.kclone.set_output(n,self.kpolyctl.cookie())
        self.__agent.s1clone.set_output(n,self.s1output.cookie())
        self.__agent.s2clone.set_output(n,self.s2output.cookie())
        self.__agent.bclone.set_output(n,self.boutput.cookie())
        self.__agent.cclone.set_output(n,self.coutput.cookie())
        self.__agent.pedal1_clone.set_output(n,self.output_pedal1.cookie())
        self.__agent.pedal2_clone.set_output(n,self.output_pedal2.cookie())
        self.__agent.pedal3_clone.set_output(n,self.output_pedal3.cookie())
        self.__agent.pedal4_clone.set_output(n,self.output_pedal4.cookie())

        self.light_output.set_output(1,self.__agent.light_switch.get_input(n))

        self.__agent.mode_selector.gate_output(n,tee,utils.make_change_nb(piw.slowchange(utils.changify(self.__setstate))))
        self.__agent.mode_selector.select(n,self.get_value())

    def unplumb(self):
        if self.__tee is not None:
            n = self.__slot
            piw.fastchange(self.__tee)(piw.makebool(False,0))
            self.__agent.mode_selector.clear_output(n)
            self.__agent.kclone.clear_output(n)
            self.__agent.s1clone.clear_output(n)
            self.__agent.s2clone.clear_output(n)
            self.__agent.bclone.clear_output(n)
            self.__agent.cclone.clear_output(n)
            self.__agent.pedal1_clone.clear_output(n)
            self.__agent.pedal2_clone.clear_output(n)
            self.__agent.pedal3_clone.clear_output(n)
            self.__agent.pedal4_clone.clear_output(n)
            self.light_output.clear_output(1)
            
            self.__tee = None
    
    def calc_keynum(self):
        return self.__agent.calc_physical_keynum(self[24].get_value(),self[25].get_value())

    def update_status_index(self):
        keynum = self.calc_keynum()
        if keynum:
            self.__agent.mode_selector.gate_status_index(self.__slot,keynum)
        

class OutputList(atom.Atom):
    def __init__(self,agent):
        self.agent = agent
        self.__plumbing = True
        atom.Atom.__init__(self,names='output',creator=self.__create,wrecker=self.__wreck,protocols='create')

    def __wreck(self,i,o):
        if self.__plumbing:
            e.unplumb()

    def rpc_listinstances(self,arg):
        return logic.render_termlist(self.keys())

    def rpc_instancename(self,a):
        return 'kgroup output'

    def rpc_createinstance(self,arg):
        slot = int(arg)
        output = self.create(slot)
        if not output:
            return async.failure('output in use')
        return output.id()

    def rpc_delinstance(self,arg):
        n = int(arg)

        for k,v in self.items():
            if n == k:
                oid = v.id()
                del self[k]
                self.__check_single()
                return oid

        return async.failure('output not in use')

    def load_state(self,state,delegate,phase):
        self.__plumbing = False

        for v in self.values():
            v.unplumb()

        atom.Atom.load_state(self,state,delegate,phase)

        for v in self.values():
            v.enable(False)
            v.plumb()
            v.enable(v.get_value())

        self.__plumbing = True
        self.__check_single()

    def __create(self,i):
        return Output(self,i)

    def create(self,slot=None):
        if slot is not None and self.has_key(slot):
            output = self[slot]
            if output:
                return output

        slot = self.find_hole()

        output = Output(self,slot)
        output.plumb()
        self[slot] = output

        self.__check_single()

        return output

    def output_selector(self,v):
        if v.is_tuple():
            keynum = v.as_tuple_value(2).as_long()
            row = int(v.as_tuple_value(1).as_tuple_value(0).as_float())
            col = int(v.as_tuple_value(1).as_tuple_value(1).as_float())
            for k,o in self.items():
                orow = o[24].get_value()
                ocol = o[25].get_value()

                select = False

                if row == orow and col == ocol:
                    select = True
                elif row == 0 and ocol == keynum:
                    select = True
                else:
                    okeynum = o.calc_keynum()
                    if okeynum == keynum:
                        select = True

                if select:
                    self.agent.mode_selector.gate_input(k)(piw.makebool_nb(True,v.time()))
                    break

    def uncreate(self,oid):
        for k,v in self.items():
            if v.id()==oid:
                del self[k]
                self.__check_single()
                return True

        return False

    def activate(self,name):
        for k,v in self.items():
            if v.id()==name:
                self.agent.mode_selector.activate(k)
                break

    def __check_single(self):
        outputs = self.values()
        if len(outputs)==1:
            outputs[0].enable(True)

    def update_status_indexes(self):
        for v in self.values():
            v.update_status_index()


class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='kgroup', container=8,ordinal=ordinal)

        self.__active_blinker = None
        self.__choices = []
        self.__coursekeys = []
        self.__course = 1
        self.__cur_size = 0

        self.__private = node.Server()
        self.set_private(self.__private)

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))

        self.mapper = piw.kgroup_mapper()

        self[15] = bundles.Output(1,False,names='light output',protocols='revconnect')
        self.lights = bundles.Splitter(self.domain,self[15])
        self.status_mapper = piw.function1(False,1,1,piw.makenull(0),self.lights.cookie())
        self.status_mapper.set_functor(self.mapper.light_filter())
        self.light_switch = piw.multiplexer(self.status_mapper.cookie(),self.domain)

        self[34] = bundles.Output(1,False,names='selection output')
        self.outputselection = bundles.Splitter(self.domain,self[34])

        self.kclone = piw.clone(False) # keys
        self.s1clone = piw.clone(False) # strip 1
        self.s2clone = piw.clone(False) # strip 2
        self.bclone = piw.clone(False) # breath
        self.cclone = piw.clone(True) # controller
        self.pedal1_clone = piw.clone(False) # pedal 1
        self.pedal2_clone = piw.clone(False) # pedal 2
        self.pedal3_clone = piw.clone(False) # pedal 3
        self.pedal4_clone = piw.clone(False) # pedal 4

        self.kclone.set_policy(False)

        self.key_mapper = piw.clone(True) # keys
        self.key_mapper.set_filtered_data_output(1, self.kclone.cookie(), self.mapper.key_filter(), 5)

        self.s1input = bundles.ScalarInput(self.s1clone.cookie(),self.domain,signals=(1,2))
        self.s2input = bundles.ScalarInput(self.s2clone.cookie(),self.domain,signals=(1,2))
        self.binput = bundles.ScalarInput(self.bclone.cookie(),self.domain,signals=(1,))
        self[4] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.s1input.nodefault_policy(1,False),names='strip position input',ordinal=1)
        self[6] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.s2input.nodefault_policy(1,False),names='strip position input',ordinal=2)
        self[5] = atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.binput.nodefault_policy(1,False),names='breath input')
        self[2] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.s1input.nodefault_policy(2,False),names='absolute strip input',ordinal=1)
        self[7] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.s2input.nodefault_policy(2,False),names='absolute strip input',ordinal=2)

        self.input_pedal1 = bundles.VectorInput(self.pedal1_clone.cookie(), self.domain, signals=(1,))
        self.input_pedal2 = bundles.VectorInput(self.pedal2_clone.cookie(), self.domain, signals=(1,))
        self.input_pedal3 = bundles.VectorInput(self.pedal3_clone.cookie(), self.domain, signals=(1,))
        self.input_pedal4 = bundles.VectorInput(self.pedal4_clone.cookie(), self.domain, signals=(1,))
        self[30] = atom.Atom(domain=domain.Aniso(), policy=self.input_pedal1.vector_policy(1,False),names='pedal input', ordinal=1)
        self[31] = atom.Atom(domain=domain.Aniso(), policy=self.input_pedal2.vector_policy(1,False),names='pedal input', ordinal=2)
        self[32] = atom.Atom(domain=domain.Aniso(), policy=self.input_pedal3.vector_policy(1,False),names='pedal input', ordinal=3)
        self[33] = atom.Atom(domain=domain.Aniso(), policy=self.input_pedal4.vector_policy(1,False),names='pedal input', ordinal=4)

        self.kinput = bundles.VectorInput(self.key_mapper.cookie(),self.domain,signals=(1,2,3,4,5),threshold=10)

        self[11] = atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.kinput.vector_policy(1,False), names='activation input')
        self[12] = atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.kinput.vector_policy(2,False), names='pressure input')
        self[13] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.kinput.vector_policy(3,False), names='roll input')
        self[14] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.kinput.vector_policy(4,False), names='yaw input')
        self[22] = atom.Atom(domain=domain.Aniso(), policy=self.kinput.vector_policy(5,False), names='key input')

        self.controller = Controller(self,self.cclone.cookie())
        self.__private[2] = self.controller.state()

        self.keypulse = piw.changelist_nb()
        self.keychoice = utils.make_change_nb(piw.slowchange(utils.changify(self.__choice)))
        self.selgate = piw.functor_backend(5,True)
        self.selgate.set_gfunctor(self.keypulse)
        self.kclone.enable(250,True)
        self.kclone.set_output(250,self.selgate.cookie())

        self.status_buffer = piw.statusbuffer(self.light_switch.gate_input(),251,self.light_switch.get_input(251))
        self.status_buffer.autosend(False)
        lightselection = self.status_buffer.enabler()
        modeselection = utils.make_change_nb(piw.slowchange(utils.changify(self.__mode_selection)))
        self.mode_selector = piw.selector(self.light_switch.get_input(250),self.outputselection.cookie(),lightselection,modeselection,250,False)

        self.modepulse = piw.changelist_nb()
        piw.changelist_connect_nb(self.modepulse,self.status_buffer.blinker())
        piw.changelist_connect_nb(self.modepulse,self.mode_selector.mode_input())

        self[23] = atom.Atom(domain=domain.Aniso(), names='mode key input', policy=policy.FastPolicy(self.modepulse,policy.FilterStreamPolicy(piw.make_d2b_nb_functor(self.__is_mode_key))))
        self[24] = atom.Atom(domain=domain.BoundedInt(-32767,32767), names='mode key row', init=None, policy=atom.default_policy(self.__change_mode_key_row))
        self[25] = atom.Atom(domain=domain.BoundedInt(-32767,32767), names='mode key column', init=None, policy=atom.default_policy(self.__change_mode_key_column))
        self[26] = atom.Atom(domain=domain.String(), init='[]', names='key map', policy=atom.default_policy(self.__set_members))
        self[28] = atom.Atom(domain=domain.String(), init='[]', names='course size', policy=atom.default_policy(self.__set_course_size))

        self[27] = atom.Atom(domain=domain.Aniso(), names='selection input', policy=policy.FastPolicy(self.mode_selector.gate_selection_input(),policy.AnisoStreamPolicy()))


        self.add_verb2(3,'set([un],None)',callback=self.__untune)

        self.add_verb2(5,'create([],None,role(None,[mass([output])]))',self.__create)
        self.add_verb2(6,'create([un],None,role(None,[concrete,singular,partof(~(a)#"1")]))', self.__uncreate)
        self.add_verb2(7,'choose([],None,role(None,[matches([k])]),option(as,[numeric]))',self.__choose)
        self.add_verb2(11,'choose([un],None)',self.__unchoose)
        
        self.add_verb2(8,'set([],None,role(None,[tagged_ideal([~server,course],[offset])]),role(to,[mass([semitone])]))',callback=self.__set_course_semi)
        self.add_verb2(9,'set([],None,role(None,[tagged_ideal([~server,course],[offset])]),role(to,[mass([interval])]))',callback=self.__set_course_int)

        self.add_verb2(12,'clear([],None,option(None,[mass([course])]))',self.__kclear)
        self.add_verb2(13,'add([],None,role(None,[mass([k])]),option(to,[mass([course])]))',self.__kadd)
        self.add_verb2(14,'add([],None,role(None,[mass([k])]),role(to,[mass([k])]),option(as,[mass([course])]))',self.__radd)
        self.add_verb2(15,'choose([],None,role(None,[mass([output])]))', self.__ochoose)

        self[9] = atom.Atom(domain=domain.BoundedFloatOrNull(-20,20),init=None,policy=atom.default_policy(self.__change_base),names='base note')

        self[16] = VirtualCourse(self.controller)

        self.__upstream_rowlen = None
        self.__upstream_size = 0

        self.cfunctor = piw.functor_backend(1,True)
        self.cinput = bundles.ScalarInput(self.cfunctor.cookie(),self.domain,signals=(1,))
        self[17] = atom.Atom(domain=domain.Aniso(), policy=self.cinput.nodefault_policy(1,False),names='controller input')
        self.cfunctor.set_functor(piw.pathnull(0),utils.make_change_nb(piw.slowchange(utils.changify(self.__upstream))))

        th=(T('inc',1),T('biginc',1),T('control','updown'))

        self[18] = atom.Atom(domain=domain.BoundedFloatOrNull(-1,9,hints=th),init=None,policy=atom.default_policy(self.__change_octave),names='octave')

        self[19] = atom.Atom(domain=domain.BoundedFloatOrNull(0,12,hints=th),init=None,policy=atom.default_policy(self.__change_tonic),names='tonic',protocols='bind set',container=(None,'tonic',self.verb_container()))
        self[19].add_verb2(1,'set([],~a,role(None,[instance(~self)]),role(to,[ideal([None,note]),singular]))',create_action=self.__tune_tonic_fast,callback=self.__tune_tonic)

        self[20] = atom.Atom(domain=domain.String(),init='',policy=atom.default_policy(self.__change_scale),names='scale',protocols='bind set',container=(None,'scale',self.verb_container()))
        self[20].add_verb2(1,'set([],~a,role(None,[instance(~self)]),role(to,[ideal([None,scale]),singular]))',create_action=self.__tune_scale_fast,callback=self.__tune_scale)
        
        self[21] = atom.Atom(domain=domain.BoundedFloatOrNull(0,20),init=0.5,policy=atom.default_policy(self.__change_blinktime),names='blink')

        self[1] = OutputList(self)
        self.outputchoice = utils.make_change_nb(piw.slowchange(utils.changify(self[1].output_selector)))

    def __is_mode_key(self,d):
        row = self[24].get_value()
        col = self[25].get_value()
        if d.is_tuple():
            key = d.as_tuple_value(1)

            geo = self.__upstream_rowlen
            if row < 0:
                rowcount = len(geo)
                row = rowcount + row + 1

            if col < 0:
                rowlen = geo[row-1].as_long()
                col = rowlen + col + 1

            if int(key.as_tuple_value(0).as_float()) == row and int(key.as_tuple_value(1).as_float()) == col:
                return True
            
        return False

    def __mode_selection(self,d):
        if d.as_long():
            piw.changelist_connect_nb(self.keypulse,self.outputchoice)
        else:
            piw.changelist_disconnect_nb(self.keypulse,self.outputchoice)

    def __change_scale(self,val):
        self[20].set_value(val)
        self.controller.thing_changed(self[20].get_data(),'scale')
        return False

    def __change_octave(self,val):
        self[18].set_value(val)
        self.controller.thing_changed(self[18].get_data(),'octave')
        return False

    def __change_tonic(self,val):
        self[19].set_value(val)
        self.controller.thing_changed(self[19].get_data(),'tonic')
        return False

    def __change_base(self,val):
        self[9].set_value(val)
        self.controller.thing_changed(self[9].get_data(),'base')
        return False

    def __change_blinktime(self,val):
        if not val:
            val = 0
        self[21].set_value(val)
        self.status_buffer.set_blink_time(float(val))
        return False

    def __change_mode_key_row(self,val):
        self[24].set_value(val)
        return False

    def __change_mode_key_column(self,val):
        self[25].set_value(val)
        return False

    def __decode(self,k):
        try:
            if not k.is_dict(): return 0

            # store the upstream row lengths
            rl = k.as_dict_lookup('rowlen')
            if rl.is_tuple():
                self.__upstream_rowlen = utils.tuple_items(rl)
                self.mapper.set_upstream_rowlen(rl)
            else:
                self.__upstream_rowlen = None
                self.mapper.set_upstream_rowlen(piw.makenull(0))

            # recalculate all key geometries based on the upstream geometry
            self.__set_physical_geo(self.__cur_mapping())

            # transform the mode key row and column in case it was set from an
            # upgraded setup that only set the key in a sequential position
            if self.__upstream_rowlen:
                mode_key_row = self[24].get_value()
                mode_key_col = self[25].get_value()

                if 0 == mode_key_row and mode_key_col > 0:
                    mode_key_row = 1

                    for i in self.__upstream_rowlen:
                        rowlen = i.as_long()
                        if mode_key_col > rowlen:
                            mode_key_row += 1
                            mode_key_col -= rowlen
                        else:
                            break

                    self.__change_mode_key_row(mode_key_row) 
                    self.__change_mode_key_column(mode_key_col) 

            # calculate the total number of keys on the keyboard
            total_keys = 0

            if self.__upstream_rowlen:
                for i in self.__upstream_rowlen:
                    total_keys += i.as_long()

            return total_keys
        except:
            return 0

    def get_size(self):
        return self.__upstream_size

    def calc_physical_keynum(self,row,col):
        if self.__upstream_rowlen:
            return calc_keynum(self.controller.gettuple('rowlen'),row,col)
        return None

    def __upstream(self,k):
        size = self.__decode(k)
        self.__upstream_size = size
        self.status_buffer.set_blink_size(size)

    def __set_course_semi(self,subject,course,interval):
        (typ,id) = action.crack_ideal(action.arg_objects(course)[0])
        course = int(id)
        semis = action.mass_quantity(interval)
        self.controller.set_course_semis(int(course),semis)
        return action.nosync_return()

    def __set_course_int(self,subject,course,interval):
        (typ,id) = action.crack_ideal(action.arg_objects(course)[0])
        course = int(id)
        ints = action.mass_quantity(interval)
        self.controller.set_course_steps(int(course),ints)
        return action.nosync_return()

    def __set_course_size(self,value):
        self.controller.set_courses(logic.parse_clause(value))
        self[28].set_value(value)

    def __tune_tonic_fast(self,ctx,subj,dummy,arg):
        type,thing = action.crack_ideal(action.arg_objects(arg)[0])
        value = int(thing)
        chg = piw.change2(self.controller.changetonic(value),piw.slowchange(utils.changify(SlowTonicChange(self,value).change)))
        return chg,None

    def __tune_scale_fast(self,ctx,subj,dummy,arg):
        type,thing = action.crack_ideal(action.arg_objects(arg)[0])
        value = action.marshal(thing)
        chg = piw.change2(self.controller.changescale(value),piw.slowchange(utils.changify(SlowScaleChange(self,value).change)))
        return chg,None

    def __tune_tonic(self,subj,dummy,arg):
        type,thing = action.crack_ideal(action.arg_objects(arg)[0])
        self[19].set_value(int(thing))
        self.controller.thing_changed(self[19].get_data(),'tonic')
        return action.nosync_return()

    def __tune_scale(self,subj,dummy,arg):
        type,thing = action.crack_ideal(action.arg_objects(arg)[0])
        self[20].set_value(action.marshal(thing))
        self.controller.thing_changed(self[20].get_data(),'scale')
        return action.nosync_return()

    def __untune(self,subj):
        self[18].set_value(None)
        self.controller.thing_changed(self[18].get_data(),'octave')

        self[19].set_value(None)
        self.controller.thing_changed(self[19].get_data(),'tonic')
       
        self[20].set_value('')
        self.controller.thing_changed(self[20].get_data(),'scale') 
        return action.nosync_return()

    def kgroup_size(self):
        return self.__cur_size

    def key_choice(self):
        return self.__choices

    def __ochoose(self,subj,name):
        name = int(action.abstract_wordlist(name)[0])
        self[1].activate(name)

    def __create(self,subj,name):
        name = int(action.abstract_wordlist(name)[0])
        o = self[1].create(name)
        return action.concrete_return(o.id())

    def __uncreate(self,subj,grp):
        a = action.concrete_object(grp)
        if self[1].uncreate(a):
            return async.success(action.removed_return(a))
        return async.success(errors.doesnt_exist('output','un create'))

    def __getkeys(self):
        mapping = self.__cur_mapping()
        courses = [i.as_long() for i in self.controller.getlist('courselen')]

        if not courses:
            courses = [len(mapping)]

        coursekeys = []
        keys = [m[0] for m in self.__cur_mapping()]
        while len(courses):
            courselen = courses[0]
            coursekeys.append(keys[:courselen])
            courses = courses[1:]
            keys = keys[courselen:]

        return coursekeys

    def __setkeys(self,coursekeys):
        while len(coursekeys)>0 and not coursekeys[-1]:
            coursekeys.pop()

        self.controller.setlist('courselen',[piw.makelong(len(k),0) for k in coursekeys])

        keys = []
        for c in coursekeys:
            keys.extend(c)

        mapping = [(k,i+1) for (i,k) in enumerate(keys)]
        self.__set_mapping(tuple(mapping))

    def __kclear(self,subject,course):
        if course is None:
            self.controller.setlist('courselen',[piw.makelong(0,0)])
            self.__set_mapping(())
            return

        course = int(action.mass_quantity(course))-1
        coursekeys = self.__getkeys()

        if len(coursekeys) > course:
            coursekeys[course] = []

        self.__setkeys(coursekeys)

        
    def __radd(self,subject,f,t,course):
        if course:
            course = int(action.mass_quantity(course))-1
        else:
            course = 0

        f = int(action.mass_quantity(f))
        t = int(action.mass_quantity(t))
        r = range(f,t+1)

        self.controller.ensure(course+1)
        coursekeys = self.__getkeys()

        while len(coursekeys) <= course:
            coursekeys.append([])

        #coursekeys[course] = []

        for i in range(0,len(coursekeys)):
            coursekeys[i] = removelist(coursekeys[i],r)

        coursekeys[course].extend(r)
        self.__setkeys(coursekeys)


    def __kadd(self,subject,k,course):
        k = int(action.mass_quantity(k))

        if course:
            course = int(action.mass_quantity(course))-1
        else:
            course = 0

        self.controller.ensure(course+1)
        coursekeys = self.__getkeys()

        while len(coursekeys) <= course:
            coursekeys.append([])

        for i in range(0,len(coursekeys)):
            if k in coursekeys[i]:
                coursekeys[i].remove(k)

        coursekeys[course].append(k)

        self.__setkeys(coursekeys)

    def __choose(self,subject,dummy,course):
        print 'start choosing..',self.__upstream_size
        self.__choices = []
        self.__course=1

        self.mapper.clear_mapping()
        self.mapper.activate_mapping()

        curmap = self.__cur_mapping() # [[from,to 1..N]]
        active = [f for (f,t) in curmap] # all active keys

        courses = [i.as_long() for i in self.controller.getlist('courselen')]
        coursekeys=[]

        if course:
            self.__course = int(action.abstract_string(course))
        else:
            # collapse current courses into 1 course
            self.__course=1
            n=0
            for c in courses:
                n=n+c
            courses=[n]
            self.controller.setlist('courselen',[piw.makelong(n,0)])

        if len(courses)>=self.__course and self.__choices==[]:
            clen=courses[self.__course-1]
            cstart=0
            for i in range(0,self.__course-1):
                cstart=cstart+courses[i]
            self.__coursekeys=active[cstart:cstart+clen] 

        for k in range(1,self.__upstream_size+1):
            if k in active:
                if k in self.__coursekeys:
                    self.status_buffer.set_status(k,const.status_choose_active)
                else:
                    self.status_buffer.set_status(k,const.status_choose_used)
            else:
                self.status_buffer.set_status(k,const.status_choose_available)

        self.status_buffer.send()

        self.status_buffer.override(True)
        self.mode_selector.choose(True)
        piw.changelist_connect_nb(self.keypulse,self.keychoice)

    def __choice(self,v):
        if not v.is_tuple() or v.as_tuplelen() != 4: return
        keynum = v.as_tuple_value(0).as_long()

        if self.__choices and keynum==self.__choices[-1]:
            self.mode_selector.choose(False)
            piw.changelist_disconnect_nb(self.keypulse,self.keychoice)
            self.status_buffer.override(False)
            self.__do_mapping()
        else:
            if not keynum in self.__choices:
                self.__choices.append(keynum)
                for k in self.__coursekeys:
                    self.status_buffer.set_status(k,const.status_choose_used)
                self.__coursekeys=[]
                self.status_buffer.set_status(keynum,const.status_choose_active)
                self.status_buffer.send()
        
    def __unchoose(self,subject):
        self.mode_selector.choose(False)
        piw.changelist_disconnect_nb(self.keypulse,self.keychoice)
        self.status_buffer.override(False)
        
    def __do_mapping(self):
        self.controller.ensure(self.__course)
        cl = [i.as_long() for i in self.controller.getlist('courselen')]

        ckeys = self.__choices
        okeys = [m[0] for m in self.__cur_mapping()]

        s=0
        no=0
        oldcourses={}
        for courselen in cl:
            no=no+1
            e=s+courselen
            oldcourses[no]=okeys[s:e]
            s=e

        remove=[]
        for k,v in oldcourses.iteritems():
            if not k ==self.__course: 
                for c in ckeys:
                    if c in v:
                       remove.append((k,c))        
        for t in remove:
            oldcourses[t[0]].remove(t[1])

        oldcourses[self.__course]=ckeys
        mapping=[]
        count=0
        for k,v in oldcourses.iteritems():
            for nk in v:
                count=count+1
                mapping.append((nk,count))
        
        cl=[]
        for v in oldcourses.itervalues():
            cl.append(len(v))
        self.controller.setlist('courselen',[piw.makelong(i,0) for i in cl])

        self.__set_mapping(tuple(mapping))
    
    def __set_physical_geo(self,mapping):
        rowbounds = None
        if self.__upstream_rowlen:
            rowbounds = []
            for rowlen in self.__upstream_rowlen:
                rowbounds.append([0,0])

        rowlen = piw.tuplenull(0)
        rowoffset = piw.tuplenull(0)

        if rowbounds:
            for (src,dst) in mapping:
                row = 0 
                col = src
                for upstream_rowlen in self.__upstream_rowlen:
                    keysinrow = upstream_rowlen.as_long()
                    if col > keysinrow:
                        row += 1
                        col -= keysinrow
                    else:
                        break

                if 0 == rowbounds[row][0]: rowbounds[row][0] = col
                else: rowbounds[row][0] = min(rowbounds[row][0], col)
                rowbounds[row][1] = max(rowbounds[row][1], col)

            for (first,last) in rowbounds: 
                if first and last:
                    rowlen = piw.tupleadd(rowlen, piw.makelong(last-first+1,0))
                if first:
                    rowoffset = piw.tupleadd(rowoffset, piw.makelong(first-1,0))
                else:
                    rowoffset = piw.tupleadd(rowoffset, piw.makenull(0))

        self.controller.settuple('rowlen',rowlen)
        self.controller.settuple('rowoffset',rowoffset)
        self.mapper.set_rowlen(rowlen)
        self.mapper.set_rowoffset(rowoffset)

        self[1].update_status_indexes()

    def __set_mapping(self,mapping):
        mapper = self.mapper
        mapper.clear_mapping()

        # set the physical to musical mapping
        for (src,dst) in mapping:
            self.mapper.set_mapping(src,dst)

        # determine the physical geometry
        self.__set_physical_geo(mapping)

        # activate the mappings
        mapper.activate_mapping()

        # store the mapping description in the state of the agent
        mapstr = logic.render_term(mapping)
        self[26].set_value(mapstr)
        self.__cur_size = len(mapping)

        print 'activating mapping'
        self.key_mapper.enable(1,False)
        self.key_mapper.enable(1,True)

    def controller_changed(self):
        self[28].set_value(logic.render_term(self.controller.get_courses()))
        self.mapper.set_courselen(self.controller.gettuple('courselen'))

    def __set_members(self,value):
        mapping = logic.parse_clause(value)
        self.__set_mapping(mapping)

    def __cur_mapping(self):
        return logic.parse_clause(self[26].get_value())


agent.main(Agent)
