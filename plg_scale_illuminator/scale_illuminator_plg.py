

import piw
from pi import agent, domain, bundles, atom, action
from pi.logic.shortcuts import T
from . import scale_illuminator_version as version

from .scale_illuminator_plg_native import scale_illuminator

IN_CONTROL=1
OUT_LIGHT=1

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        #
        agent.Agent.__init__(self, signature=version, names='scale illuminator', ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()

        self.domain.set_source(piw.makestring('*',0))

        self[1] = atom.Atom(names='outputs')
        self[1][1] = bundles.Output(OUT_LIGHT, False, names='light output', protocols='revconnect')
        self.output = bundles.Splitter(self.domain, self[1][1])
        self.illuminator = scale_illuminator(self.domain, self.output.cookie())
        
        th=(T('stageinc',1),T('inc',1),T('biginc',1),T('control','updown'))
        sh=(T('choices','[0,2,4,5,7,9,11,12]','[0,1,2,3,4,5,6,7,8,9,10,11,12]','[0,2,4,6,8,10,12]','[0,2,3,5,7,8,10,12]','[0,3,5,6,7,10,12]', '[0,2,3,6,7,8,11,12]','[0,3,5,7,10,12]','[0,2,4,7,9,12]'), T('control','selector'))

        self.control_input = bundles.VectorInput(self.illuminator.cookie(), self.domain, signals=(1,))
        self[2] = atom.Atom(names="inputs")
        self[2][1] =atom.Atom(domain=domain.Aniso(),policy=self.control_input.vector_policy(1,False),names='controller input')
        self[2][2] = atom.Atom(domain=domain.String(hints=sh), policy=atom.default_policy(self.__change_scale), names='scale',protocols='bind set',container=(None,'scale',self.verb_container()))
        self[2][2].add_verb2(1,'set([],~a,role(None,[instance(~self)]),role(to,[ideal([None,scale]),singular]))',callback=self.__tune_scale)
        self[2][3] = atom.Atom(domain=domain.BoundedFloatOrNull(0,12,hints=th),init=None,policy=atom.default_policy(self.__change_tonic),names='tonic',protocols='bind set',container=(None,'tonic',self.verb_container()))
        self[2][3].add_verb2(1,'set([],~a,role(None,[instance(~self)]),role(to,[ideal([None,note]),singular]))',callback=self.__tune_tonic)
        self[2][4] = atom.Atom(domain=domain.Bool(),init=False,policy=atom.default_policy(self.__change_inverted),names='inverted')
        self[2][5] = atom.Atom(domain=domain.Bool(),init=True,policy=atom.default_policy(self.__change_root_light),names='root')

        self.add_verb2(3,'clear([],None,role(None,[matches([scale])]))', callback=self.__clear_scale )
         
    def __tune_scale(self,subj,dummy,arg):
        print 'tune scale',arg
        type,thing = action.crack_ideal(action.arg_objects(arg)[0])
        scale = action.marshal(thing)
        self[2][2].get_policy().set_value(scale)
        self.illuminator.reference_scale(scale);
        return action.nosync_return()

    def __clear_scale(self,subject,name):
        print 'clear scale'
        self[2][2].set_value('')
        self.illuminator.reference_scale('');
        return True
        

    def __change_scale(self,value):
        print 'change scale',value
        self[2][2].set_value(value)
        self.illuminator.reference_scale(value);
        return True

    def __tune_tonic(self,subj,dummy,arg):
        print 'tune tonic',arg
        type,thing = action.crack_ideal(action.arg_objects(arg)[0])
        tonic = int(thing)
        self[2][3].get_policy().set_value(tonic)
        self.illuminator.reference_tonic(tonic);
        return action.nosync_return()

    def __change_tonic(self,value):
        print 'change tonic',value
        self[2][3].set_value(value)
        self.illuminator.reference_tonic(value);
        return True
                
    def __change_inverted(self,value):
        self[2][4].set_value(value)
        self.illuminator.inverted(value);
        return True
    
    def __change_root_light(self,value):
        self[2][5].set_value(value)
        self.illuminator.root_light(value);
        return True


#
# Define Agent as this agents top level class
#
agent.main(Agent)
