
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
from pi import agent, atom, bundles, domain, async, action, upgrade, policy
from pi.logic.shortcuts import T
from plg_synth import stereo_mixer_version as version
import synth_native

num_inputs = 24
db_range = 70

def volume_function(f):
    if f<=0.01: return 0.0
    fn = f/100.0
    db = db_range*(1.0-fn)
    sc = pow(10.0,-db/20.0)
    return sc


def pan_function(f):
    return (f+1.0)/2.0

class Input(agent.Agent):
    def __init__(self,index,vol,pan,clk,cookie,sig):
        agent.Agent.__init__(self,names='mixer channel',ordinal=index, protocols='bind is_subsys notagent',subsystem='channel',signature=version)

        self.channel_mixer = piw.stereomixer(vol,pan,clk,cookie)
        self.aggregator = piw.aggregator(self.channel_mixer.cookie(),clk)

        self.ctl_input = bundles.ScalarInput(self.aggregator.get_output(1),clk,signals=(1,2))
        self.audio_input = bundles.VectorInput(self.aggregator.get_output(2),clk,signals=(1,2))

        self[1] = atom.Atom(domain=domain.BoundedFloat(-1,1), init=0, names='left audio input', policy=self.audio_input.vector_policy(1,True), protocols='obm')
        self[2] = atom.Atom(domain=domain.BoundedFloat(-1,1), init=0, names='right audio input', policy=self.audio_input.vector_policy(2,True), protocols='obm')
        self[3] = atom.Atom(domain=domain.BoundedFloat(0,120,hints=(T('inc',1),T('biginc',10),T('control','updown'))), init=50, names='volume', policy=self.ctl_input.policy(1,policy.LopassStreamPolicy(1000,0.97)), protocols='bind input')
        self[4] = atom.Atom(domain=domain.BoundedFloat(-1,1,hints=(T('inc',0.02),T('biginc',0.2),T('control','updown'))), init=0, names='pan', policy=self.ctl_input.policy(2,policy.LopassStreamPolicy(1000,0.97)), protocols='bind input')

    def inuse(self):
        return self[1].is_connected() or self[2].is_connected()

    def disconnect(self):
        self[1].clear_connections()
        self[2].clear_connections()

class Agent(agent.Agent):

    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='stereo mixer', protocols='inputlist has_subsys', icon='plg_synth/mixer.png',ordinal=ordinal)

        self.clk = piw.clockdomain_ctl()
        self.vol = piw.make_f2f_table(0,120,1000,picross.make_f2f_functor(volume_function))
        self.pan = piw.make_f2f_table(-1,1,1000,picross.make_f2f_functor(pan_function))

        self[1] = atom.Atom()
        self[1][1] = bundles.Output(1, True, names='left audio output')
        self[1][2] = bundles.Output(2, True, names='right audio output')

        self.output = bundles.Splitter(self.clk, self[1][1], self[1][2])
        self.final_mixer = piw.stereomixer(self.vol,self.pan,self.clk,self.output.cookie())
        self.aggregator = piw.aggregator(self.final_mixer.cookie(),self.clk)

        self.ctl_input = bundles.ScalarInput(self.aggregator.get_output(1),self.clk,signals=(1,2))

        self[2] = atom.Atom()
        self[2][1] = atom.Atom(domain=domain.BoundedFloat(0,120,hints=(T('inc',1),T('biginc',10),T('control','updown'))),
                            init=100, names='master volume',
                            policy=self.ctl_input.policy(1,policy.LopassStreamPolicy(1000,0.97)))
        self[2][2] = atom.Atom(domain=domain.BoundedFloat(-1,1,hints=(T('inc',0.02),T('biginc',0.2),T('control','updown'))),
                            init=0, names='master pan',
                            policy=self.ctl_input.policy(2,policy.LopassStreamPolicy(1000,0.97)))

        for n in range(0,num_inputs):
            ss = Input(n+1,self.vol,self.pan,self.clk,self.aggregator.get_output(n+2),self.signature())
            self.add_subsystem(str(n),ss)

    def rpc_addinput(self,dummy):
        for (k,v) in self.iter_subsys_items():
            if not v.inuse():
                return async.success(action.marshal((v.id(),False)))

        return async.failure('no free mixer channels')

    def rpc_delinput(self,id):
        for (k,v) in self.iter_subsys_items():
            if id=='' or v.id()==id:
                v.disconnect()

    def rpc_lstinput(self,dummy):
        r=tuple([s.id() for (k,s) in self.iter_subsys_items() if s.inuse()])
        return async.success(action.marshal(r))

class Upgrader(upgrade.Upgrader):
    def upgrade_3_0_to_4_0(self,tools,address):
        root = tools.root(address)

        for ss in tools.get_subsystems(address):
            print 'upgrading',ss
            ssr = tools.root(ss)
            ssr.ensure_node(255,17)

        return True

agent.main(Agent,Upgrader)

