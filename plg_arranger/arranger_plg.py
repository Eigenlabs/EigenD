
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

from pi import agent,atom,bundles,domain,errors,policy,utils,action,const,node,upgrade,logic
from plg_arranger import arranger_version as version

import piw
import arranger_native

class RowTarget(atom.Atom):
    def __init__(self,agent,row=None):
        self.__agent = agent
        self.__fastdata = bundles.FastSender()
        atom.Atom.__init__(self,domain=domain.Aniso(),policy=policy.FastReadOnlyPolicy())
        self.get_policy().set_source(self.__fastdata)
        self.get_policy().set_clock(self.__agent.model.get_clock())

        self.set_private(node.static(_1=node.Server(change=self.__changetarget),_2=node.Server(value=piw.makelong(1,0),change=self.__changerefs)))
        self.target = self.get_private()[1]
        self.refs = self.get_private()[2]

        if row is not None:
            self.__changetarget(piw.makelong(row,0))

    def __changetarget(self,d):
        if d.is_long():
            self.target.set_data(d)
            self.__agent.model.set_target(d.as_long(),self.__fastdata.sender())

    def __changerefs(self,d):
        if d.is_long():
            self.refs.set_data(d)

    def addref(self):
        r = self.refs.get_data().as_long()
        self.refs.set_data(piw.makelong(r+1,0))

    def delref(self):
        r = self.refs.get_data().as_long()
        if r==1:
            return True
        self.refs.set_data(piw.makelong(r-1,0))
        return False

    def clear(self):
        d = self.target.get_data()
        if d.is_long():
            self.__agent.model.clear_target(d.as_long())


class Event(node.Server):
    def __init__(self,list,data=None):
        self.__list = list
        node.Server.__init__(self,change=utils.changify(self.__list.state_changed))
        if data is not None:
            self.set_data(data)


class EventList(node.Server):
    def __init__(self,agent,data=None):
        self.__agent = agent
        node.Server.__init__(self,creator=self.__create)
        self.__setter = piw.fastchange(self.__agent.model.set_event())
        self.__agent.model.event_set(piw.make_change_nb(utils.slowchange(self.model_changed)))

    def __create(self,k):
        return Event(self)

    def state_changed(self,d):
        if d.is_string():
            e = self.__encode(d.as_string())
            self.__setter(e)

    def model_changed(self,d):
        r,c,e = self.__decode(d)
        if e is not None:
            self.__setup(r,c,e)
        else:
            self.__remove(r,c)

    def __setup(self,r,c,e):
        print 'setup',r,c,e
        x = self.__find(r,c,True)
        if x is None:
            k = self.find_hole()
            if k:
                self[k] = Event(self,piw.makestring('%d,%d,%f'%(r,c,e),0))
        else:
            k,v = x
            v.set_data(piw.makestring('%d,%d,%f'%(r,c,e),0))

    def __remove(self,r,c):
        while True:
            x = self.__find(r,c,False)
            if x is None:
                return
            k,v = x
            del self[k]

    def __find(self,r,c,find_null):
        print 'find',r,c
        for k,v in self.iteritems():
            if find_null and not v.get_data().is_string():
                return k,v
            r2,c2,_ = self.__split(v.get_data().as_string())
            if r==r2 and c==c2:
                return k,v
        return None

    def __split(self,str):
        bits = str.split(',')
        return int(bits[0]),int(bits[1]),float(bits[2])

    def __encode(self,str):
        r,c,e = self.__split(str)
        t = (c<<32)|r
        return piw.makefloat_nb(e,t)

    def __decode(self,d):
        if d.is_float():
            e = d.as_float()
        else:
            e = None
        t = d.time()
        m = (2**32)-1
        r = t&m
        t >>= 32
        c = t&m
        return r,c,e


class Parameters(atom.Atom):
    def __init__(self,agent):
        self.__agent = agent
        atom.Atom.__init__(self)

        self[1] = atom.Atom(domain=domain.BoundedInt(1,10000),names='beginning',policy=atom.default_policy(self.__start_change),container=(None,'loopstart',self.__agent.verb_container()),protocols='set')
        self[1].add_verb2(1,'set([],~a,role(None,[instance(~self)]),role(to,[numeric]))',create_action=self.__start_create,clock=True)
        self.__agent.model.loopstart_set(piw.make_change_nb(utils.slowchange(self.__start_set)))

        self[2] = atom.Atom(domain=domain.BoundedInt(1,10000),init=16,names='end',policy=atom.default_policy(self.__end_change),container=(None,'loopend',self.__agent.verb_container()),protocols='set')
        self[2].add_verb2(1,'set([],~a,role(None,[instance(~self)]),role(to,[numeric]))',create_action=self.__end_create,clock=True)
        self.__agent.model.loopend_set(piw.make_change_nb(utils.slowchange(self.__end_set)))

        self[3] = atom.Atom(domain=domain.BoundedFloat(1,100),init=1,names="step",policy=atom.default_policy(self.__step_change),container=(None,'step',self.__agent.verb_container()),protocols='set')
        self[3].add_verb2(1,'set([],~a,role(None,[instance(~self)]),role(to,[numeric]))',create_action=self.__step_create,clock=True)
        self.__agent.model.stepnumerator_set(piw.make_change_nb(utils.slowchange(self.__step_set)))

        self[4] = atom.Atom(domain=domain.BoundedFloat(1,100),init=2,names="fraction",policy=atom.default_policy(self.__fraction_change),container=(None,'fraction',self.__agent.verb_container()),protocols='set')
        self[4].add_verb2(1,'set([],~a,role(None,[instance(~self)]),role(to,[numeric]))',create_action=self.__fraction_create,clock=True)
        self.__agent.model.stepdenominator_set(piw.make_change_nb(utils.slowchange(self.__fraction_set)))

        self[5] = atom.Atom(names="position",container=(None,'position',self.__agent.verb_container()),protocols='set')
        self[5].add_verb2(1,'set([],~a,role(None,[instance(~self)]),role(to,[numeric]))',create_action=self.__position_create,clock=True)

        self[6] = atom.Atom(domain=domain.BoundedFloat(0.5,20),init=0.5,names='doubletap',policy=atom.default_policy(self.__doubletap_change))
        self.__agent.view.doubletap_set(piw.make_change_nb(utils.slowchange(self.__doubletap_set)))

    def __start_change(self,v):
        fc = piw.fastchange(self.__agent.model.set_loopstart())
        fc(piw.makelong_nb(v-1,0))
        return True

    def __start_create(self,ctx,subj,dummy,arg):
        v = int(action.abstract_string(arg))
        if v<1 or v>10000:
            print 'loopstart out of range' 
            return async.success(errors.out_of_range('1 to 10000','set'))
        return piw.trigger(self.__agent.model.set_loopstart(),piw.makelong_nb(v-1,0)),None

    def __start_set(self,d):
        self[1].get_policy().set_value(d.as_long()+1)

    def __end_change(self,v):
        fc = piw.fastchange(self.__agent.model.set_loopend())
        fc(piw.makelong_nb(v-1,0))
        return True

    def __end_create(self,ctx,subj,dummy,arg):
        v = int(action.abstract_string(arg))
        if v<1 or v>10000:
            print 'loopend out of range'
            return async.success(errors.out_of_range('1 to 10000','set'))
        return piw.trigger(self.__agent.model.set_loopend(),piw.makelong_nb(v-1,0)),None

    def __end_set(self,d):
        self[2].get_policy().set_value(d.as_long()+1)

    def __step_change(self,v):
        fc = piw.fastchange(self.__agent.model.set_stepnumerator())
        fc(piw.makefloat_nb(v,0))
        return True

    def __step_create(self,ctx,subj,dummy,arg):
        v = float(action.abstract_string(arg))
        if v<1 or v>100:
            print 'step out of range'
            return async.success(errors.out_of_range('1 to 100','set'))
        return piw.trigger(self.__agent.model.set_stepnumerator(),piw.makefloat_nb(v,0)),None

    def __step_set(self,d):
        self[3].get_policy().set_value(d.as_float())

    def __fraction_change(self,v):
        fc = piw.fastchange(self.__agent.model.set_stepdenominator())
        fc(piw.makefloat_nb(v,0))
        return True

    def __fraction_create(self,ctx,subj,dummy,arg):
        v = float(action.abstract_string(arg))
        if v<1 or v>100:
            print 'fraction out of range'
            return async.success(errors.out_of_range('1 to 100', 'set'))
        return piw.trigger(self.__agent.model.set_stepdenominator(),piw.makefloat_nb(v,0)),None

    def __fraction_set(self,d):
        self[4].get_policy().set_value(d.as_float())

    def __position_create(self,ctx,subj,dummy,arg):
        v = int(action.abstract_string(arg))
        if v<1 or v>10000:
            print 'position out of range'
            return async.success(errors.out_of_range('1 to 10000', 'set'))
        return piw.trigger(self.__agent.model.set_position(),piw.makelong_nb(v-1,0)),None

    def __doubletap_change(self,v):
        fc = piw.fastchange(self.__agent.view.set_doubletap())
        fc(piw.makefloat_nb(v,0))
        return True

    def __doubletap_set(self,d):
        self[6].get_policy().set_value(d.as_float())

class Agent(agent.Agent):
    def __init__(self,address,ordinal):
        self.domain = piw.clockdomain_ctl()
        vc = atom.VerbContainer(clock_domain=self.domain)

        agent.Agent.__init__(self,signature=version,names='arranger', protocols='bind', container=(9,'agent',vc), ordinal=ordinal)

        self[1] = self.verb_container()

        self.model = arranger_native.model(self.domain)

        self.light_output = bundles.Output(1,False, names='light output',protocols='revconnect')
        self.light_splitter = bundles.Splitter(self.domain, self.light_output)
        self.light_convertor = piw.lightconvertor(self.light_splitter.cookie())
        self.view = arranger_native.view(self.model,self.light_convertor.cookie())

        self.ctlr_fb = piw.functor_backend(1,True)
        self.ctlr_fb.set_functor(piw.pathnull(0),self.view.control())
        self.ctlr_input = bundles.ScalarInput(self.ctlr_fb.cookie(),self.domain,signals=(1,))
        self[2] = atom.Atom(domain=domain.Aniso(),policy=self.ctlr_input.policy(1,False),names='controller input')

        self.kinput = bundles.VectorInput(self.view.cookie(),self.domain,signals=(1,2,3,5))
        self[3] = atom.Atom()
        self[3][1] = atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.kinput.vector_policy(1,False),names='pressure input',protocols='nostage')
        self[3][2] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.kinput.merge_policy(2,False),names='roll input',protocols='nostage')
        self[3][3] = atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.kinput.merge_policy(3,False),names='yaw input',protocols='nostage')
        self[3][5] = atom.Atom(domain=domain.Aniso(), policy=self.kinput.vector_policy(5,False),names='key input')
        self[3][4] = self.light_output

        self.cinput = bundles.ScalarInput(self.model.cookie(),self.domain,signals=(1,2))
        self.cinput.add_upstream(self.verb_container().clock)
        self[4] = atom.Atom()
        self[4][1] = atom.Atom(domain=domain.Aniso(), policy=self.cinput.nodefault_policy(1,False),names='song beat input')
        self[4][2] = atom.Atom(domain=domain.Aniso(), policy=self.cinput.nodefault_policy(2,False),names='running input')

        self[5] = atom.Atom(creator=self.__createtarget,wrecker=self.__wrecktarget)
        self.add_mode2(1,'mode([],role(when,[numeric,singular]),option(using,[instance(~server)]))', self.__mode, self.__query, self.__cancel_mode)

        self[7] = Parameters(self)

        self.__eventlist = EventList(self)
        self.__playstop = node.Server(change=self.__play_change)
        self.__playstop[1] = self.__eventlist
        
        self.set_private(self.__playstop)

        self.add_verb2(1,'play([],~self)',create_action=self.__play,clock=True)
        self.add_verb2(2,'play([un],~self)',create_action=self.__unplay,clock=True)
        self.add_verb2(3,'cancel([],~self,role(None,[numeric,singular]))',self.__cancel_verb)
        self.add_verb2(4,'clear([],~self)',self.__clear_verb)
        self.model.playstop_set(piw.make_change_nb(utils.slowchange(self.__play_set)))

    def __play(self,*args):
        return piw.trigger(self.model.set_playstop(),piw.makebool_nb(True,0)),None

    def __unplay(self,*args):
        return piw.trigger(self.model.set_playstop(),piw.makebool_nb(False,0)),None

    def __play_change(self,d):
        if d.is_bool():
            fc = piw.fastchange(self.model.set_playstop())
            fc(d)

    def __play_set(self,d):
        self.get_private().set_data(d)

    def __createtarget(self,k):
        return RowTarget(self)

    def __wrecktarget(self,k,v):
        v.clear()

    def __mode(self,text,k,u):
        row = int(action.abstract_string(k))-1

        for v in self[5].itervalues():
            d = v.target.get_data()
            if d.is_long() and d.as_long()==row:
                v.addref()
                return logic.render_term((v.id(),('transient',)))

        i = self[5].find_hole()
        if i:
            self[5][i] = RowTarget(self,row)
            return logic.render_term((self[5][i].id(),('transient',)))

        return None

    def __query(self,k,u):
        print '__query',k,u
        row = int(action.abstract_string(k))-1

        for v in self[5].values():
            d = v.target.get_data()
            if d.is_long() and d.as_long()==row:
                return [ v.id() ]

        return []

    def __cancel_verb(self,subj,row):
        row = int(action.abstract_string(row))-1
        print 'cancelling row',row

        for k,v in self[5].iteritems():
            d = v.target.get_data()
            if d.is_long() and d.as_long()==row:
                id = v.id()
                v.clear()
                del self[5][k]
                return action.cancel_return(self.id(),1,id)

    def __clear_verb(self,subj):
        self.view.clear_events()
        
    def __cancel_mode(self,id):
        print '__cancel',id
        for k,v in self[5].iteritems():
            if id==v.id():
                if v.delref():
                    v.clear()
                    del self[5][k]
                return

class Upgrader(upgrade.Upgrader):
    def upgrade_1_0_0_to_1_0_1(self,tools,address):
        print 'upgrading arranger',address
        root = tools.get_root(address)

        root.ensure_node(3,5).set_name('key input')

    def phase2_1_0_1(self,tools,address):
        root = tools.get_root(address)
        root.mimic_connections((3,1),(3,5),'key output')

    def upgrade_2_0_to_3_0(self,tools,address):
        root = tools.root(address)
        evts = root.get_node(6)
        nevents = root.ensure_node(255,6,1)

        if evts:
            for child in evts.iter():
                p = child.path[-1]
                nevents.ensure_node(p).set_data(child.get_data())

        return True

    def upgrade_1_0_to_2_0(self,tools,address):
        root = tools.root(address)
        evts = root.get_node(6)
        if evts:
            for child in evts.iter():
                d = child.get_data()
                if d.is_float():
                    e = d.as_float()
                    t = d.time()
                    m = (2**32)-1
                    r = t&m
                    t >>= 32
                    c = t&m
                    child.set_string('%d,%d,%f'%(r,c,e))
        return True

    def upgrade_0_0_to_1_0(self,tools,address):
        root = tools.root(address)

        loopstart = piw.makelong(1+root.get_node(7,255,6,1).get_data().as_long(),0)
        loopend = piw.makelong(1+root.get_node(7,255,6,2).get_data().as_long(),0)
        step = root.get_node(8,1)
        frac = root.get_node(8,2)

        root.remove(7)
        root.remove(8)

        root.ensure_node(7,1,255,1).set_string('bint(1,10000,1,[])')
        root.ensure_node(7,1,255,8).set_string('loopstart')
        root.ensure_node(7,1,255,17)
        root.ensure_node(7,1,255,18)
        root.ensure_node(7,1,254).set_data(loopstart)

        root.ensure_node(7,2,255,1).set_string('bint(1,10000,1,[])')
        root.ensure_node(7,2,255,8).set_string('loopend')
        root.ensure_node(7,2,255,17)
        root.ensure_node(7,2,255,18)
        root.ensure_node(7,2,254).set_data(loopend)

        root.ensure_node(7,3).copy(step)
        root.ensure_node(7,3,255,17)
        root.ensure_node(7,3,255,18)

        root.ensure_node(7,4).copy(frac)
        root.ensure_node(7,4,255,17)
        root.ensure_node(7,4,255,18)

        root.remove(8)

        root.ensure_node(255,6).set_data(piw.makebool(True,0))

        return True

agent.main(Agent,Upgrader)

