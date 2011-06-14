
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

from pi import agent,bundles,atom,action,domain,paths,upgrade,const,policy,node
from plg_simple import scaler_version as version
import piw
import picross
from pi.logic.shortcuts import T

linear = lambda: piw.make_f2f_identity()
quadratic = lambda: piw.make_f2f_table(-1,1,1000,picross.make_f2f_functor(lambda x: x*x if x>0 else -x*x))
cubic = lambda: piw.make_f2f_table(-1,1,1000,picross.make_f2f_functor(lambda x: x*x*x))
step = lambda: piw.step_bucket()

curves = (linear,quadratic,cubic,step)

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='scaler',protocols='bind set',icon='plg_simple/scaler.png',container=5,ordinal=ordinal)
        self.domain = piw.clockdomain_ctl()
        self.domain.set_source(piw.makestring('*',0))

        self.__fixed = False
        self.set_private(node.Server(value=piw.makebool(False,0),change=self.__changefix))

        self[1] = atom.Atom(names='outputs')
        self[1][1] = bundles.Output(1,False,names='activation output', protocols='')
        self[1][2] = bundles.Output(2,False,names='pressure output', protocols='')
        self[1][3] = bundles.Output(3,False,names='roll output', protocols='')
        self[1][4] = bundles.Output(4,False,names='yaw output', protocols='')
        self[1][5] = bundles.Output(5,False,names='scale note output', protocols='')
        self[1][6] = bundles.Output(6,False,names='frequency output', protocols='')

        self.ctl = piw.scaler_controller()
        self.ctl_input = bundles.VectorInput(self.ctl.cookie(),self.domain,signals=(1,))

        self.output = bundles.Splitter(self.domain,*self[1].values())
        self.filter = piw.scaler(self.ctl,self.output.cookie(),cubic())
        self.input = bundles.VectorInput(self.filter.cookie(), self.domain,signals=(1,2,3,4,5,6,7,9,10,11,12,13,14,15,16,17))
        self.input.correlator.clocksink().add_upstream(self.ctl_input.correlator.clocksink())

        self[4]=atom.Atom(names='inputs')

        self[4][1]=atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.input.merge_policy(1,False),names='activation input')
        self[4][2]=atom.Atom(domain=domain.BoundedFloat(0,1),policy=self.input.vector_policy(2,False),names='pressure input')
        self[4][3]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.input.merge_policy(3,False),names='roll input')
        self[4][4]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.input.merge_policy(4,False),names='yaw input')

        th=(T('inc',1),T('biginc',1),T('control','updown'))
        bh=(T('inc',1),T('biginc',1),T('control','updown'))
        sh=(T('choices','[0,2,4,5,7,9,11,12]','[0,1,2,3,4,5,6,7,8,9,10,11,12]','[0,2,4,6,8,10,12]','[0,2,3,5,7,8,10,12]','[0,3,5,6,7,10,12]', '[0,2,3,6,7,8,11,12]','[0,3,5,7,10,12]','[0,2,4,7,9,12]'), T('control','selector'))
        self[4][5]=atom.Atom(domain=domain.BoundedFloat(0,12,hints=th),policy=self.input.merge_policy(5,False),names='tonic input',protocols='bind set',container=(None,'tonic',self.verb_container()))
        self[4][6]=atom.Atom(domain=domain.BoundedFloat(-20,20,hints=bh),policy=self.input.merge_policy(6,False),names='base note input',protocols='bind')


        self[4][7]=atom.Atom(domain=domain.String(hints=sh),init='[0,2,4,5,7,9,11,12]',policy=self.input.merge_policy(7,False),names='scale input',protocols='bind set',container=(None,'scale',self.verb_container()))
        self[4][8]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.input.merge_policy(9,policy.LopassStreamPolicy(200,0.6)),names='k pitch bend input')
        self[4][9]=atom.Atom(domain=domain.BoundedFloat(-1,1),policy=self.input.merge_policy(10,False),names='global pitch bend input')
        self[4][10]=atom.Atom(domain=domain.BoundedFloat(0,72),init=1,policy=self.input.merge_policy(11,False),names='k bend range input',protocols='bind')
        self[4][11]=atom.Atom(domain=domain.BoundedFloat(0,72),init=12,policy=self.input.merge_policy(12,False),names='global bend range input',protocols='bind')
        self[4][12]=atom.Atom(domain=domain.Aniso(),policy=self.ctl_input.vector_policy(1,False),names='controller input')
        self[4][13]=atom.Atom(domain=domain.Bool(),policy=self.input.merge_policy(14,False),names='override',protocols='bind')
        self[4][14]=atom.Atom(domain=domain.BoundedFloat(-1,9,hints=th),init=3,policy=self.input.merge_policy(15,False),names='octave input',protocols='bind',container=(None,'octave',self.verb_container()))
        self[4][15]=atom.Atom(domain=domain.BoundedInt(1,4),init=2,policy=atom.default_policy(self.__set_curve),names='curve',protocols='bind')
        self[4][16]=atom.Atom(domain=domain.BoundedFloat(0,1000),policy=self.input.merge_nodefault_policy(16,False),names='key input')
        self[4][17]=atom.Atom(domain=domain.BoundedFloat(-10,10,hints=th),init=0,policy=self.input.merge_policy(17,False),names='relative octave input',protocols='bind')

        self.add_verb2(3,'choose([],None,role(none,[ideal([None,scale]),singular]))',callback=self.__tune_scale)
        self.add_verb2(5,'fix([],None)',callback=self.__fix)
        self.add_verb2(6,'fix([un],None)',callback=self.__unfix)

        self[4][5].add_verb2(1,'set([],~a,role(None,[instance(~self)]),role(to,[ideal([None,note]),singular]))',callback=self.__tune_tonic)
        self[4][7].add_verb2(2,'set([],~a,role(None,[instance(~self)]),role(to,[ideal([None,scale]),singular]))',callback=self.__tune_scale)


    def __tune_tonic(self,subj,dummy,arg):
        print 'set tonic',arg
        type,thing = action.crack_ideal(action.arg_objects(arg)[0])
        tonic = int(thing)
        if not self.__fixed: self[4][5].get_policy().set_value(tonic)
        return action.nosync_return()

    def __tune_scale(self,subj,dummy,arg):
        print 'set scale',arg
        type,thing = action.crack_ideal(action.arg_objects(arg)[0])
        if not self.__fixed: self[4][7].get_policy().set_value(thing)
        return action.nosync_return()

    def __set_curve(self,c):
        i = int(c)
        self.filter.set_bend_curve(curves[i-1]())
        return True

    def __fix(self,subject):
        self.__fixed=True
        self.get_private().set_data(piw.makebool(True,0))
        return action.nosync_return()

    def __unfix(self,subject):
        self.__fixed=False
        self.get_private().set_data(piw.makebool(False,0))
        return action.nosync_return()

    def __changefix(self,d):
        if d.is_bool():
            self.__fixed=d.as_bool()
            self.get_private().set_data(d)

class Upgrader(upgrade.Upgrader):
    def upgrade_7_0_to_8_0(self,tools,address):
        root = tools.root(address)
        rnode = root.ensure_node(4,17,255)
        rnode.ensure_node(1).set_data(piw.makestring('',0))
        rnode.ensure_node(8).set_data(piw.makestring('relative octave input',0))
        root.ensure_node(4,17,254).set_data(piw.makelong(0,0))
        return True

    def upgrade_4_0_to_5_0(self,tools,address):
        root = tools.root(address)
        root.ensure_node(4,5,255,17)
        root.ensure_node(4,7,255,17)
        root.ensure_node(4,14,255,17)
        return True

    def upgrade_5_0_to_6_0(self,tools,address):
        root = tools.root(address)
        root.ensure_node(255,6).set_data(piw.makebool(False,0))
        return True

    def upgrade_6_0_to_7_0(self,tools,address):
        root = tools.root(address)
        root.ensure_node(5).erase_children()
        return True

agent.main(Agent,Upgrader)
