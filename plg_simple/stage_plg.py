
from pi import agent,bundles,atom,action,domain,paths,upgrade,const,policy,node
from . import stage_version as version
import piw
import picross
from pi.logic.shortcuts import T

class Widget(atom.Atom):
    def __init__(self):
        atom.Atom.__init__(self,domain=domain.StringEnum('roll','yaw','pressure'),names='Test View',transient=True,policy=atom.default_policy(self.__changed),protocols="widget-combo")

    def __changed(self,value):
        print 'received',value

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='stage',ordinal=ordinal)

        self[1]=Widget();

agent.main(Agent)
