
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
    def __init__(self,cookie):
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

    def thing_changed(self,d,kk):
        self.__dict.put(kk,d)

    def set_course_semis(self,idx,val):
        self.ensure(idx)
        l = map(float,self.getlist('courseoffset'))
        l[idx-1] = float(val)
        self.setlist('courseoffset',l)

    def set_course_steps(self,idx,val):
        self.ensure(idx)
        l = map(float,self.getlist('courseoffset'))
        l[idx-1] = float(val+10000 if val else 0)
        self.setlist('courseoffset',l)

    def num_courses(self):
        l = self.getlist('courseoffset')
        return len(l) if l else 0

    def ensure(self,course):
        l = map(float,self.getlist('courseoffset'))
        if course>len(l):
            xl = course-len(l)
            l.extend([0]*xl)
            self.setlist('courseoffset',l)

        l = map(int,self.getlist('courselen'))
        if course>len(l):
            xl = course-len(l)
            l.extend([0]*xl)
            self.setlist('courselen',l)

    def setlist(self,name,l):
        self.__dict.put_ctl(name, piw.makestring(repr(l),piw.tsd_time()))
        self.__state.set_data(self.__dict.get_ctl_dict())

    def getlist(self,name):
        d = self.__dict.get_ctl_value(name)
        if d is None or not d.is_string():
            return []
        s = d.as_string()
        if len(s)<2:
            return []
        return s[1:-1].split(',')

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
        self.__tee = None
        self.__current = None

        atom.Atom.__init__(self,domain=domain.Bool(),init=False,policy=atom.default_policy(self.enable),names='kgroup output',ordinal=slot,container=const.verb_node)

        self.light_output = piw.clone(True)
        self.input = bundles.VectorInput(self.light_output.cookie(),self.__agent.domain,signals=(1,))

        self[1] = bundles.Output(1,False, names='activation output')
        self[2] = bundles.Output(2,False, names='pressure output')
        self[3] = bundles.Output(3,False, names='roll output')
        self[4] = bundles.Output(4,False, names='yaw output')
        self[5] = bundles.Output(1,False, names='strip position output',ordinal=1)
        self[9] = bundles.Output(1,False, names='strip position output',ordinal=2)
        self[6] = bundles.Output(1,False, names='breath output')
        self[7] = bundles.Output(1,False, names='controller output', continuous=True)
        self[8] = atom.Atom(names='light input',protocols='revconnect',policy=self.input.vector_policy(1,False,clocked=False,auto_slot=True),domain=domain.Aniso())
        self[10] = bundles.Output(2,False, names='absolute strip output',ordinal=1)
        self[11] = bundles.Output(2,False, names='absolute strip output',ordinal=2)
        self[30] = bundles.Output(1,False,names='pedal output', ordinal=1, protocols='')
        self[31] = bundles.Output(1,False,names='pedal output', ordinal=2, protocols='')
        self[32] = bundles.Output(1,False,names='pedal output', ordinal=3, protocols='')
        self[33] = bundles.Output(1,False,names='pedal output', ordinal=4, protocols='')

        self[20] = VirtualKey(self.__agent.kgroup_size,self.__agent.key_choice)

        self.koutput = bundles.Splitter(self.__agent.domain,self[1],self[2],self[3],self[4])
        self.s1output = bundles.Splitter(self.__agent.domain,self[5],self[10])
        self.s2output = bundles.Splitter(self.__agent.domain,self[9],self[11])
        self.boutput = bundles.Splitter(self.__agent.domain,self[6])
        self.coutput = bundles.Splitter(self.__agent.domain,self[7])
        self.output_pedal1 = bundles.Splitter(self.__agent.domain,self[30])
        self.output_pedal2 = bundles.Splitter(self.__agent.domain,self[31])
        self.output_pedal3 = bundles.Splitter(self.__agent.domain,self[32])
        self.output_pedal4 = bundles.Splitter(self.__agent.domain,self[33])

        self.kpolyctl = piw.polyctl(10,self.koutput.cookie(),False,5)

    def property_change(self,key,value):
        if key=='ordinal' and value and value.is_long() and value.as_long():
            if self.__current:
                self.__list.rename(self,self.__current,value.as_long())

    def __setstate(self,data):
        self.set_value(data.as_norm()!=0.0)

    def enable(self,val):
        if self.__current:
            self.__agent.mode_selector.select(self.__current-1,val)
        return True

    def plumb(self,name):
        self.unplumb()

        self.__current = name
        tee = piw.changelist_nb()
        self.__tee = tee

        n = self.__current

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

        self.__agent.mode_selector.gate_output(n-1,tee,utils.make_change_nb(piw.slowchange(utils.changify(self.__setstate))))
        self.__agent.selgate.set_functor(piw.pathone(n,0),self.__agent.mode_selector.gate_input(n-1))
        self.__agent.mode_selector.select(self.__current-1,self.get_value())


    def unplumb(self):
        if self.__current is not None:
            n = self.__current
            piw.fastchange(self.__tee)(piw.makebool_nb(False,0))
            self.__agent.mode_selector.clear_output(n-1)
            self.__agent.selgate.clear_functor(piw.pathone_nb(n,0))
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
            self.__current = None
            
            self.__tee = None

class OutputList(atom.Atom):
    def __init__(self,agent):
        self.agent = agent
        self.__plumbing = True
        atom.Atom.__init__(self,names='output',creator=self.__create,wrecker=self.__wreck,protocols='create')

    def __wreck(self,i,o):
        if self.__plumbing:
            e.unplumb()

    def rpc_listinstances(self,arg):
        i = [ self[i].get_property_long('ordinal',0) for i in self ]
        return logic.render_termlist(i)

    def rpc_createinstance(self,arg):
        name = int(arg)
        output = self.create(name)
        if not output:
            return async.failure('output in use')
        return output.id()

    def rpc_delinstance(self,arg):
        name = int(arg)

        for k,v in self.items():
            o = v.get_property_long('ordinal',0)
            if o == name:
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
            o = v.get_property_long('ordinal',0)
            v.enable(False)
            if o:
                v.plumb(o)
                v.enable(v.get_value())

        self.__plumbing = True
        self.__check_single()

    def __create(self,i):
        return Output(self,i)

    def create(self,name):
        for v in self.values():
            o = v.get_property_long('ordinal',0)
            if o==name:
                return None

        slot = self.find_hole()
        self[slot] = Output(self,slot)
        self[slot].set_property_long('ordinal',name,notify=False)
        self[slot].plumb(name)
        self.__check_single()

        return self[slot]

    def uncreate(self,oid):
        for k,v in self.items():
            if v.id()==oid:
                del self[k]
                self.__check_single()
                return True

        return False

    def rename(self,output,old_name,new_name):
        if old_name == new_name:
            return

        output.unplumb()

        for k,v in self.items():
            o = v.get_property_long('ordinal',0)
            if o == new_name:
                v.set_property_long('ordinal',old_name,notify=False)
                v.plumb(old_name)
                
        output.set_property_long('ordinal',new_name,notify=False)
        output.plumb(new_name)

    def move(self,old_id,new_name):
        for k,v in self.items():
            if v.id()==old_id:
                old_name = v.get_property_long('ordinal',0)
                self.rename(v,old_name,new_name)
                return True

        return False

    def __check_single(self):
        outputs = self.values()
        if len(outputs)==1:
            outputs[0].enable(True)


class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='kgroup', icon='plg_simple/kgroup.png',container=8,ordinal=ordinal)

        self.__active_blinker = None
        self.__choices = []
        self.__coursekeys=[]
        self.__course=1
        self.__cur_size = 0

        self.__private = node.Server()
        self.set_private(self.__private)
        self.__private[3] = node.Server(value=piw.makestring('[]',0),change=self.__set_members)

        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))

        self.mapper = piw.kgroup_mapper()

        self[15] = bundles.Output(1,False, names='light output',protocols='revconnect')
        self.lights = bundles.Splitter(self.domain,self[15])
        self.status_mapper = piw.function1(False,1,1,piw.makenull(0),self.lights.cookie())
        self.status_mapper.set_functor(self.mapper.light_filter())
        self.light_switch = piw.multiplexer(self.status_mapper.cookie(),self.domain)

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
        self.key_mapper.set_filtered_output(1,self.kclone.cookie(),self.mapper.key_filter())

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
        self[30]=atom.Atom(domain=domain.Aniso(), policy=self.input_pedal1.vector_policy(1,False),names='pedal input', ordinal=1)
        self[31]=atom.Atom(domain=domain.Aniso(), policy=self.input_pedal2.vector_policy(1,False),names='pedal input', ordinal=2)
        self[32]=atom.Atom(domain=domain.Aniso(), policy=self.input_pedal3.vector_policy(1,False),names='pedal input', ordinal=3)
        self[33]=atom.Atom(domain=domain.Aniso(), policy=self.input_pedal4.vector_policy(1,False),names='pedal input', ordinal=4)

        self.kinput = bundles.VectorInput(self.key_mapper.cookie(),self.domain,signals=(1,2,3,4),threshold=10)

        self[11] = atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.kinput.vector_policy(1,False), names='activation input')
        self[12] = atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.kinput.vector_policy(2,False), names='pressure input')
        self[13] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.kinput.vector_policy(3,False), names='roll input')
        self[14] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.kinput.vector_policy(4,False), names='yaw input')


        self.controller = Controller(self.cclone.cookie())
        self.__private[2]=self.controller.state()

        self.selgate = piw.functor_backend(1,True)
        self.kclone.enable(250,True)
        self.kclone.set_output(250,self.selgate.cookie())

        self.status_buffer = piw.statusbuffer(self.light_switch.gate_input(),251,self.light_switch.get_input(251))
        self.status_buffer.autosend(False)
        self.mode_selector = piw.selector(self.light_switch.get_input(250),self.status_buffer.enabler(),250,False)

        self.modepulse = piw.changelist_nb()
        piw.changelist_connect_nb(self.modepulse,self.status_buffer.blinker())
        piw.changelist_connect_nb(self.modepulse,self.mode_selector.mode_input())

        self[10] = atom.Atom(domain=domain.Bool(), init=False, fuzzy="activation", names='mode input', protocols='explicit', policy=policy.FastPolicy(self.modepulse,policy.TriggerStreamPolicy()))

        self.add_verb2(3,'set([un],None)',callback=self.__untune)

        self.add_verb2(5,'create([],None,role(None,[mass([output])]))',self.__create)
        self.add_verb2(6,'create([un],None,role(None,[concrete,singular,partof(~(a)#"1")]))', self.__uncreate)
        self.add_verb2(7,'choose([],None,role(None,[matches([k])]),option(as,[numeric]))',self.__choose)
        
        self.add_verb2(8,'set([],None,role(None,[tagged_ideal([~server,course],[offset])]),role(to,[mass([semitone])]))',callback=self.__set_course_semi)
        self.add_verb2(9,'set([],None,role(None,[tagged_ideal([~server,course],[offset])]),role(to,[mass([interval])]))',callback=self.__set_course_int)
        self.add_verb2(10,'move([],None,role(None,[concrete,singular,partof(~(a)"#1")]),role(to,[numeric]))', self.__move)

        self.add_verb2(12,'clear([],None,option(None,[mass([course])]))',self.__kclear)
        self.add_verb2(13,'add([],None,role(None,[mass([k])]),option(to,[mass([course])]))',self.__kadd)
        self.add_verb2(14,'add([],None,role(None,[mass([k])]),role(to,[mass([k])]),option(as,[mass([course])]))',self.__radd)
        self.add_verb2(15,'choose([],None,role(None,[mass([output])]))', self.__ochoose)

        self[9] = atom.Atom(domain=domain.BoundedFloatOrNull(-20,20),init=None,policy=atom.default_policy(self.__change_base),names='base note')

        self[16] = VirtualCourse(self.controller)

        self.__upstream_size = 0

        self.cfunctor = piw.functor_backend(1,True)
        self.cinput = bundles.ScalarInput(self.cfunctor.cookie(),self.domain,signals=(1,))
        self[17] = atom.Atom(domain=domain.Aniso(), policy=self.cinput.nodefault_policy(1,False),names='controller input')
        self.cfunctor.set_functor(piw.pathnull(0),utils.make_change_nb(piw.slowchange(utils.changify(self.__upstream))))

        th=(T('inc',1),T('biginc',1),T('control','updown'))

        self[18]=atom.Atom(domain=domain.BoundedFloatOrNull(-1,9,hints=th),init=None,policy=atom.default_policy(self.__change_octave),names='octave')

        self[19]=atom.Atom(domain=domain.BoundedFloatOrNull(0,12,hints=th),init=None,policy=atom.default_policy(self.__change_tonic),names='tonic',protocols='bind set',container=(None,'tonic',self.verb_container()))
        self[19].add_verb2(1,'set([],~a,role(None,[instance(~self)]),role(to,[ideal([None,note]),singular]))',create_action=self.__tune_tonic_fast,callback=self.__tune_tonic)

        self[20]=atom.Atom(domain=domain.String(),init='',policy=atom.default_policy(self.__change_scale),names='scale',protocols='bind set',container=(None,'scale',self.verb_container()))
        self[20].add_verb2(1,'set([],~a,role(None,[instance(~self)]),role(to,[ideal([None,scale]),singular]))',create_action=self.__tune_scale_fast,callback=self.__tune_scale)
        
        self[21]=atom.Atom(domain=domain.BoundedFloatOrNull(0,20),init=0.5,policy=atom.default_policy(self.__change_blinktime),names='blink')

        self[1]=OutputList(self)

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
        self[21].set_value(val)
        self.status_buffer.set_blink_time(float(val))
        return False

    def __decode(self,k):
        try:
            if not k.is_dict(): return 0
            cl = k.as_dict_lookup('courselen')
            if not cl.is_string(): return 0
            cll = logic.parse_clause(cl.as_string())
            if not isinstance(cll,tuple): return 0
            k=0
            for i in cll: k+=int(i)
            return k
        except:
            return 0

    def get_size(self):
        return self.__upstream_size

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
        self.mode_selector.activate(name-1)

    def __create(self,subj,name):
        name = int(action.abstract_wordlist(name)[0])
        o = self[1].create(name)
        return action.concrete_return(o.id())


    def __uncreate(self,subj,grp):
        a = action.concrete_object(grp)
        if self[1].uncreate(a):
            return async.success(action.removed_return(a))
        return async.success(errors.doesnt_exist('output','un create'))

    def __move(self,subj,src,dst):
        old = action.concrete_object(src)
        new = int(action.abstract_wordlist(dst)[0])

        if self[1].move(old,new):
            return async.success()

        return async.success(errors.doesnt_exist('output','move'))

    def __getkeys(self):
        mapping = self.__cur_mapping()
        courses = map(int,self.controller.getlist('courselen'))

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

        self.controller.setlist('courselen',[len(k) for k in coursekeys])

        keys = []
        for c in coursekeys:
            keys.extend(c)

        mapping = [(k,str(i+1)) for (i,k) in enumerate(keys)]
        self.__set_mapping(tuple(mapping))

    def __kclear(self,subject,course):
        if course is None:
            self.controller.setlist('courselen',[0])
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
        r = map(str,range(f,t+1))

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
        k = str(int(action.mass_quantity(k)))

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
        active = [str(f) for (f,t) in curmap] # all active keys

        courses=self.controller.getlist('courselen')
        coursekeys=[]

        if course:
            self.__course = int(action.abstract_string(course))
        else:
            # collapse current courses into 1 course
            self.__course=1
            n=0
            for c in map(int,courses):
                n=n+c
            courses=[str(n)]
            self.controller.setlist('courselen',[n])

        if len(courses)>=self.__course and self.__choices==[]:
            clen=int(courses[self.__course-1])
            cstart=0
            for i in range(0,self.__course-1):
                cstart=cstart+int(courses[i])
            self.__coursekeys=active[cstart:cstart+clen] 

        for k in range(1,self.__upstream_size+1):
            if str(k) in active:
                if str(k) in self.__coursekeys:
                    self.status_buffer.set_status(k,const.status_choose_active)
                else:
                    self.status_buffer.set_status(k,const.status_choose_used)
            else:
                self.status_buffer.set_status(k,const.status_choose_available)

        self.status_buffer.send()

        self.status_buffer.override(True)
        self.mode_selector.choose(True)
        self.selgate.set_efunctor(piw.slowchange(utils.changify(self.__choice)))

    def __choice(self,v):
        v=int(str(v))

        if self.__choices and v==self.__choices[-1]:
            self.mode_selector.choose(False)
            self.selgate.clear_efunctor()
            self.status_buffer.override(False)
            self.__do_mapping()
        else:
            if not v in self.__choices:
                self.__choices.append(v)
                for k in self.__coursekeys:
                    self.status_buffer.set_status(int(k),const.status_choose_used)
                self.__coursekeys=[]
                self.status_buffer.set_status(v,const.status_choose_active)
                self.status_buffer.send()
        
    def __do_mapping(self):
        self.controller.ensure(self.__course)
        cl = map(int,self.controller.getlist('courselen'))

        ckeys = [str(k) for k in self.__choices]
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
                mapping.append((nk,str(count)))
        
        self.__set_mapping(tuple(mapping))
        cl=[]
        for v in oldcourses.itervalues():
            cl.append(len(v))
        self.controller.setlist('courselen',cl)

    def __set_mapping(self,mapping):
        mapper = self.mapper
        mapper.clear_mapping()

        for (src,dst) in mapping:
            mapper.set_mapping(int(src),int(dst))

        mapper.activate_mapping()
        mapstr = logic.render_term(mapping)
        self.__private[3].set_data(piw.makestring(mapstr,0))
        self.__cur_size = len(mapping)

        print 'activating mapping'
        self.key_mapper.enable(1,False)
        self.key_mapper.enable(1,True)


    def __set_members(self,value):
        if value.is_string():
            mapping=logic.parse_clause(value.as_string())
            self.__set_mapping(mapping)

    def __cur_mapping(self):
        return logic.parse_clause(self.__private[3].get_data().as_string())


class Upgrader(upgrade.Upgrader):

    def ssname(self,myaddress,index):
        (myid,mypath) = guid.split(myaddress)
        nid = ('%s/%s%s' % (myid,index,mypath))[:26]
        return '<%s>' % nid

    def upgrade_1_0_1_to_1_0_2(self,tools,address):
        root = tools.get_root(address)

        outputs = {}
        for i in root.get_node(255,1).iter():
            output_slot = i.path[-1]
            output_name = int(i.get_data().as_string())
            old_address = self.ssname(address,output_slot)
            old_node = tools.get_root(old_address)
            new_node = root.ensure_node(1,output_slot)
            new_address = new_node.id()
            print 'moving kgroup output',old_address,'to',new_address
            if old_node:
                old_state = old_node.get_tree()
                new_node.set_tree(old_state)
                new_node.set_meta_long('ordinal',output_name)
                tools.move_connections(old_address,new_address)

        outputs = tools.get_subsystems(address)
        for i in outputs:
            tools.delete_agent(i)

        tools.invalidate_connections()

    def upgrade_1_0_0_to_1_0_1(self,tools,address):
        root = tools.get_root(address)
        existing_o = set(tools.get_subsystems(address))
        actual_o = set([self.ssname(address,i.path[-1]) for i in root.get_node(255,1).iter()])

        for o in existing_o:
            if not o in actual_o:
                print 'removing extraneous kgroup output',o
                tools.delete_agent(o)

    def upgrade_14_0_to_15_0(self,tools,address):
        root = tools.root(address)

        pressure_conn = logic.parse_clauselist(root.ensure_node(12,255,2).get_string_default(''))

        p1_conn = []
        p2_conn = []
        p3_conn = []
        p4_conn = []

        for p in pressure_conn:
            id,path = paths.breakid_list(p.args[2])

            if 'alpha' in id and path == [2]:
                p1_conn.append(T('conn',p.args[0],p.args[1],paths.makeid_list(id,12),p.args[3]))
                p2_conn.append(T('conn',p.args[0],p.args[1],paths.makeid_list(id,13),p.args[3]))
                p3_conn.append(T('conn',p.args[0],p.args[1],paths.makeid_list(id,14),p.args[3]))
                p4_conn.append(T('conn',p.args[0],p.args[1],paths.makeid_list(id,15),p.args[3]))

            if 'kgroup' in id and path == [2]:
                p1_conn.append(T('conn',p.args[0],p.args[1],paths.makeid_list(id,30),p.args[3]))
                p2_conn.append(T('conn',p.args[0],p.args[1],paths.makeid_list(id,31),p.args[3]))
                p3_conn.append(T('conn',p.args[0],p.args[1],paths.makeid_list(id,32),p.args[3]))
                p4_conn.append(T('conn',p.args[0],p.args[1],paths.makeid_list(id,33),p.args[3]))

        root.ensure_node(30,255,2).set_data(piw.makestring(logic.render_termlist(p1_conn),0))
        root.ensure_node(31,255,2).set_data(piw.makestring(logic.render_termlist(p2_conn),0))
        root.ensure_node(32,255,2).set_data(piw.makestring(logic.render_termlist(p3_conn),0))
        root.ensure_node(33,255,2).set_data(piw.makestring(logic.render_termlist(p4_conn),0))

        return True

    def upgrade_13_0_to_14_0(self,tools,address):
        root = tools.root(address)
        n18 = root.ensure_node(18,254)
        if n18.get_data().is_float() and n18.get_data().as_float()==0: n18.set_data(piw.makenull(0))
        n19 = root.ensure_node(19,254)
        if n19.get_data().is_float() and n19.get_data().as_float()==0: n19.set_data(piw.makenull(0))
        n9 = root.ensure_node(9,254)
        if n9.get_data().is_float() and n9.get_data().as_float()==0: n9.set_data(piw.makenull(0))
        return True

    def upgrade_12_0_to_13_0(self,tools,address):
        for ss in tools.get_subsystems(address):
            ssr = tools.root(ss)
            ssr.ensure_node(32,255,1)
            ssr.ensure_node(32,255,3)
            ssr.ensure_node(32,255,9)
            ssr.ensure_node(33,255,1)
            ssr.ensure_node(33,255,3)
            ssr.ensure_node(33,255,9)
        return True

    def upgrade_11_0_to_12_0(self,tools,address):
        root = tools.root(address)
        keys = [ int(v.get_string()) for v in root.ensure_node(255,6,1).iter() ]
        root.get_node(255,6,1).erase()
        for k in keys:
            n = root.ensure_node(255,6,1,k)
            n.set_string(str(k),piw.tsd_time())
        return True

    def upgrade_10_0_to_11_0(self,tools,address):
        root = tools.root(address)

        root.ensure_node(30).setname(names="pedal input",ordinal=1)
        root.ensure_node(31).setname(names="pedal input",ordinal=2)
        root.ensure_node(32).setname(names="pedal input",ordinal=3)
        root.ensure_node(33).setname(names="pedal input",ordinal=4)

        for ss in tools.get_subsystems(address):
            ssr = tools.root(ss)
            ssr.ensure_node(30).setname(names="pedal output",ordinal=1)
            ssr.ensure_node(31).setname(names="pedal output",ordinal=2)
            ssr.ensure_node(32).setname(names="pedal output",ordinal=3)
            ssr.ensure_node(33).setname(names="pedal output",ordinal=4)

        pressure_conn = logic.parse_clauselist(root.ensure_node(12,255,2).get_string_default(''))
        p1_conn = []
        p2_conn = []
        p3_conn = []
        p4_conn = []

        for p in pressure_conn:
            id,path = paths.breakid_list(p.args[2])

            if 'alpha' in id and path == [2]:
                p1_conn.append(T('conn',p.args[0],p.args[1],paths.makeid_list(id,12),p.args[3]))
                p2_conn.append(T('conn',p.args[0],p.args[1],paths.makeid_list(id,13),p.args[3]))
                p3_conn.append(T('conn',p.args[0],p.args[1],paths.makeid_list(id,14),p.args[3]))
                p4_conn.append(T('conn',p.args[0],p.args[1],paths.makeid_list(id,15),p.args[3]))

            if 'kgroup' in id and path == [2]:
                p1_conn.append(T('conn',p.args[0],p.args[1],paths.makeid_list(id,30),p.args[3]))
                p2_conn.append(T('conn',p.args[0],p.args[1],paths.makeid_list(id,31),p.args[3]))
                p3_conn.append(T('conn',p.args[0],p.args[1],paths.makeid_list(id,32),p.args[3]))
                p4_conn.append(T('conn',p.args[0],p.args[1],paths.makeid_list(id,33),p.args[3]))

        root.ensure_node(30,255,2).set_data(piw.makestring(logic.render_termlist(p1_conn),0))
        root.ensure_node(31,255,2).set_data(piw.makestring(logic.render_termlist(p2_conn),0))
        root.ensure_node(32,255,2).set_data(piw.makestring(logic.render_termlist(p3_conn),0))
        root.ensure_node(33,255,2).set_data(piw.makestring(logic.render_termlist(p4_conn),0))

        return True

    def upgrade_9_0_to_10_0(self,tools,address):
        root = tools.root(address)
        root.ensure_node(30).rename(names="pedal input")
        root.ensure_node(31).rename(names="pedal input")
        for ss in tools.get_subsystems(address):
            ssr = tools.root(ss)
            ssr.ensure_node(30).rename(names="pedal output")
            ssr.ensure_node(31).rename(names="pedal output")
        return True

    def upgrade_8_0_to_9_0(self,tools,address):
        root = tools.root(address)
        root.ensure_node(8).erase_children()
        return True

    def upgrade_7_0_to_8_0(self,tools,address):
        root = tools.root(address)

        
        pressure_conn = logic.parse_clauselist(root.ensure_node(12,255,2).get_string_default(''))
        ctl_conn = []

        for p in pressure_conn:
            id,path = paths.breakid_list(p.args[2])
            if 'keyboard' in id and path == [2]:
                ctl = paths.makeid_list(id,9)
                ctl_conn.append(T('conn',p.args[0],p.args[1],ctl,p.args[3]))
            if 'kgroup' in id and path == [2]:
                ctl = paths.makeid_list(id,7)
                ctl_conn.append(T('conn',p.args[0],p.args[1],ctl,p.args[3]))
            if 'ukbd' in id and path == [2]:
                ctl = paths.makeid_list(id,9)
                ctl_conn.append(T('conn',p.args[0],p.args[1],ctl,p.args[3]))

        cnode = root.ensure_node(17,255)
        cnode.ensure_node(2).set_data(piw.makestring(logic.render_termlist(ctl_conn),0))
        cnode.ensure_node(1).set_data(piw.makestring('aniso([])',0))
        cnode.ensure_node(8).set_data(piw.makestring('controller input',0))

        d=root.ensure_node(255,6,2)
        k=d.get_data()
        if not k.is_dict():return 0

        dict={}
        for x in ('tonic','scale','base','courseoffset','courselen','octave'):
            v = k.as_dict_lookup(x)
            if v.is_null(): dict.pop(x,None)
            else: dict[x]=v

        changedict=False
        if 'base' in dict:
            changedict=True
            del dict['base']
            # there is already an atom for base so don't do anything else

        if 'octave' in dict:
            changedict=True
            oct=(dict['octave']).as_float()
            del dict['octave']
            root.ensure_node(18,254).set_data(piw.makefloat_bounded(9,-1,0,oct,0))
        else:
            root.ensure_node(18,254).set_data(piw.makenull(0))


        if 'tonic' in dict:
            changedict=True
            tonic=(dict['tonic']).as_float()
            del dict['tonic']
            root.ensure_node(19,254).set_data(piw.makefloat_bounded(12,0,0,tonic,0))
        else:
            root.ensure_node(19,254).set_data(piw.makenull(0))

        if 'scale' in dict:
            changedict=True
            scale=(dict['scale']).as_string()
            del dict['scale']
            root.ensure_node(20,254).set_data(piw.makestring(scale,0))
        else:
            root.ensure_node(20,254).set_data(piw.makestring('',0))
        
        if not 'courselen' in dict:
            changedict=True
            cm=logic.parse_clause(root.ensure_node(255,6,3).get_string_default(''))
            courselen=len(cm)
            dict['courselen']=piw.makestring('[%d]' % courselen,0) 

        if not 'courseoffset' in dict:
            changedict=True
            dict['courseoffset']=piw.makestring('[0]',0)

        if changedict:
            nd=utils.makedict_nb(dict,0)
            root.ensure_node(255,6,2).set_data(nd)

        return True

    def upgrade_6_0_to_7_0(self,tools,address):
        root = tools.root(address)
        root.ensure_node(4,255,7).set_data(piw.makelong(1,0))
        root.ensure_node(6,255,7).set_data(piw.makelong(2,0))

        for ss in tools.get_subsystems(address):
            ssr = tools.root(ss)
            ssr.ensure_node(5,255,7).set_data(piw.makelong(1,0))
            ssr.ensure_node(9,255,7).set_data(piw.makelong(2,0))

        return True

    def upgrade_5_0_to_6_0(self,tools,address):
        root = tools.root(address)
        inputs = root.get_node(2)

        act_id=set()
        pressure_id=set()
        roll_id=set()
        yaw_id=set()
        mapping = []

        for i in inputs.iter():
            pths=set()

            self.__idpath(i,1,act_id,pths)
            self.__idpath(i,2,pressure_id,pths)
            self.__idpath(i,3,roll_id,pths)
            self.__idpath(i,4,yaw_id,pths)

            if len(pths)!=1:
                return False

            pths=pths.pop()
            mapping.append( ( str(pths) , str(i.path[-1])))

        if len(act_id)==0:
            return True

        act_id=act_id.pop()
        pressure_id=pressure_id.pop()
        roll_id=roll_id.pop()
        yaw_id=yaw_id.pop()
        mapping = logic.render_term(tuple(mapping))

        inputs.erase()
        root.ensure_node(255,6,3).set_string(mapping)
        self.__setconn(root,11,act_id,'activation input','bfloat(0,1,0,[])')
        self.__setconn(root,12,pressure_id,'pressure input','bfloat(0,1,0,[])')
        self.__setconn(root,13,roll_id,'roll input','bfloat(-1,1,0,[])')
        self.__setconn(root,14,yaw_id,'yaw input','bfloat(-1,1,0,[])')

        tools.scratchpad(address)['src'] = paths.breakid_list(act_id)[0]

        return True

    def upgrade_4_0_to_5_0(self,tools,address):
        for ss in tools.get_subsystems(address):
            ssr = tools.root(ss)
            enabled = ssr.get_node(255,7).get_data().as_long()==1
            ssr.ensure_node(254).set_data(piw.makebool(enabled,0))
        return True

    @staticmethod
    def __idpath(node,a,ids,paths):
        c = node.get_node(a,255,2)
        if c is not None:
            t = logic.parse_clauselist(c.get_string())
            for tt in t:
                ids.add(tt.args[2])
                paths.add(tt.args[3])

    @staticmethod
    def __setconn(root,a,id,name,dom):
        conn = logic.render_term(T('conn',None,None,id,None))
        root.ensure_node(a,255,2).set_string(conn)
        root.ensure_node(a,255,1).set_string(dom)
        root.ensure_node(a,255,8).set_string(name)


    @staticmethod
    def __filterconn(id,connstr):
        r = []
        ct = logic.parse_termlist(connstr)
        for cn in ct:
            if paths.id2server(cn.args[2])!=id:
                r.append(cn)
        return r

    def phase2_6_0(self,tools,address):
        src = tools.scratchpad(address).get('src')
        if src is None:
            return True

        conn = T('conn',None,None,"%s#15"%address,None)

        li = tools.root(src).find_byname('light','input')
        if li is not None:
            cnc = li.ensure_node(255,2)
            cn = cnc.get_string_default('')
            cn = self.__filterconn(address,cn)
            cn.append(conn)
            cnc.set_string(logic.render_termlist(cn))

        return True

agent.main(Agent,Upgrader)
