
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

from pi import agent,atom,bundles,domain,errors,policy,utils,action,const,node,upgrade,logic,async,collection,talker
from plg_arranger import arranger_version as version

import piw
import arranger_native

class RowTargetEvent(talker.Talker):
    def __init__(self,target,fast,index):
        self.__target = target
        self.__index = index

        talker.Talker.__init__(self,self.__target.agent.finder,fast,None,names='event',ordinal=index,connection_index=None,protocols='remove')

    def property_change(self,key,value):
        if key=='help' and value and value.is_string():
            self.__target.agent.update()

    def describe(self):
        return self.get_property_string('help')

class RowTarget(collection.Collection):
    def __init__(self,agent,index):
        self.agent = agent
        self.__fastdata = bundles.FastSender()
        collection.Collection.__init__(self,domain=domain.Aniso(),policy=policy.FastReadOnlyPolicy(),creator=self.__create,wrecker=self.__wreck,ordinal=index,names='row',protocols='hidden-connection remove')
        self.get_policy().set_source(self.__fastdata)
        self.get_policy().set_clock(self.agent.model.get_clock())

        self.set_private(node.static(_1=node.Server(change=self.__changetarget)))
        self.target = self.get_private()[1]

        if index is not None:
            self.__changetarget(piw.makelong(index-1,0))

    def __changetarget(self,d):
        if d.is_long():
            self.target.set_data(d)
            self.agent.model.set_target(d.as_long(),self.__fastdata.sender())

    def clear(self):
        self.cancel_event()
        d = self.target.get_data()
        if d.is_long():
            self.agent.model.clear_target(d.as_long())

    def __create(self,i):
        self.agent.update()
        return RowTargetEvent(self,self.__fastdata,i)

    def __wreck(self,k,v):
        self.agent.update()

    @async.coroutine('internal error')
    def instance_create(self,name):
        e = RowTargetEvent(self,self.__fastdata,name)
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

    @async.coroutine('internal error')
    def create_event(self,text,called=None):
        if called:
            if called in self:
                yield async.Coroutine.failure('phrase exists')
            index = called
        else:
            index = self.find_hole()

        print 'create event on row',self.id()

        e = RowTargetEvent(self,self.__fastdata,index)
        self[index] = e
        e.attached()
        r = e.set_phrase(text)
        yield r

    @async.coroutine('internal error')
    def cancel_event(self,called=None):
        for c,e in self.items():
            if not called or called==c:
                del self[c]
                r = e.clear_phrase()
                yield r


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
        return piw.makefloat(e,t)

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
        fc(piw.makelong(v-1,0))
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
        fc(piw.makelong(v-1,0))
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
        fc(piw.makefloat(v,0))
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
        fc(piw.makefloat(v,0))
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
        fc(piw.makefloat(v,0))
        return True

    def __doubletap_set(self,d):
        self[6].get_policy().set_value(d.as_float())

class Agent(agent.Agent):
    def __init__(self,address,ordinal):
        self.domain = piw.clockdomain_ctl()
        vc = atom.VerbContainer(clock_domain=self.domain)

        self.finder = talker.TalkerFinder()

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

        self[5] = collection.Collection(creator=self.__createtarget,wrecker=self.__wrecktarget,names="row",inst_creator=self.__createtarget_inst,inst_wrecker=self.__wrecktarget_inst,protocols='hidden-connection')

        self[7] = Parameters(self)

        self.__eventlist = EventList(self)
        self.__playstop = node.Server(change=self.__play_change)
        self.__playstop[1] = self.__eventlist
        
        self.set_private(self.__playstop)

        self.add_verb2(1,'play([],~self)',create_action=self.__play,clock=True)
        self.add_verb2(2,'play([un],~self)',create_action=self.__unplay,clock=True)
        self.add_verb2(3,'cancel([],~self,role(None,[numeric,singular]),option(called,[singular,numeric]))',self.__cancel_verb)
        self.add_verb2(4,'clear([],~self)',self.__clear_verb)
        self.add_verb2(5,'do([],~self,role(None,[abstract]),role(when,[singular,numeric]),option(called,[singular,numeric]))', self.__do_verb)
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

    def __createtarget(self,r):
        return RowTarget(self,r)

    def __wrecktarget(self,r,v):
        v.clear()

    @async.coroutine('internal error')
    def __createtarget_inst(self,r):
        e = RowTarget(self,r)
        self[5][r] = e
        yield async.Coroutine.success(e)

    @async.coroutine('internal error')
    def __wrecktarget_inst(self,k,e,name):
        r = e.clear()
        yield r
        yield async.Coroutine.success()

    @async.coroutine('internal error')
    def __do_verb(self,subject,phrase,row,c):
        phrase = action.abstract_string(phrase)
        row = int(action.abstract_string(row))
        c = int(action.abstract_string(c)) if c else None
       
        target = None

        if self[5].has_key(row):
            target = self[5][row]

        if target is None:
            target = RowTarget(self,row)
            self[5][row] = target

        if c and c in self[5][row]:
            yield async.Coroutine.success(action.error_return('name in use','','do'))

        e = target.create_event(phrase,c)
        yield e
        yield async.Coroutine.success()

    @async.coroutine('internal error')
    def __cancel_verb(self,subj,row,c):
        row = int(action.abstract_string(row))
        c = int(action.abstract_string(c)) if c else None

        if row not in self[5]:
            yield async.Coroutine.success()

        if c:
            if c not in self[5][row]:
                yield async.Coroutine.success()

            r = self[5][row].cancel_event(c)
            yield r
            yield async.Coroutine.success()
        else:
            v = self[5][row]
            id = v.id()
            v.clear()
            del self[5][row]
            yield async.Coroutine.success()

    def __clear_verb(self,subj):
        self.view.clear_events()

    def update(self):
        pass

agent.main(Agent)

