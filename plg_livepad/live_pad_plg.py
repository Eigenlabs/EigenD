

import piw
from pi import agent, domain, bundles,atom
from . import live_pad_version as version

from .livepad_plg_native import live_pad

OUT_LIGHT=1

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        #
        agent.Agent.__init__(self, signature=version, names='live pad', ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()

        self.domain.set_source(piw.makestring('*',0))

        self[1] = atom.Atom(names='outputs')
        self[1][1] = bundles.Output(OUT_LIGHT, False, names='light output', protocols='revconnect')
        self.output = bundles.Splitter(self.domain, self[1][1])
        self.live = live_pad(self.domain, self.output.cookie(),"localhost", "9000","9001")


        self.key_input = bundles.VectorInput(self.live.cookie(), self.domain, signals=(1,2,3,4))
        self[2] = atom.Atom(names="inputs")
        self[2][1] = atom.Atom(domain=domain.Aniso(), policy=self.key_input.vector_policy(1,False), names='key input')
        self[2][2] = atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.key_input.vector_policy(2,False), names='pressure input')
        self[2][3] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.key_input.vector_policy(3,False), names='roll input')
        self[2][4] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.key_input.vector_policy(4,False), names='yaw input')
        
        self.left = 1;
        self.width = 50;
        self.top =1;
        self.height = 25;
        
        self[3] = atom.Atom(names='view')
        self[3][1] = atom.Atom(domain=domain.BoundedInt(1,100), policy=atom.default_policy(self.__top), init=self.top, names='top')
        self[3][2] = atom.Atom(domain=domain.BoundedInt(1,100), policy=atom.default_policy(self.__height), init=self.height, names='height')
        self[3][3] = atom.Atom(domain=domain.BoundedInt(1,100), policy=atom.default_policy(self.__left), init=self.left, names='left')
        self[3][4] = atom.Atom(domain=domain.BoundedInt(1,100), policy=atom.default_policy(self.__width), init=self.width, names='width')
        
        self[4] = atom.Atom(domain=domain.BoundedInt(0,100), policy=atom.default_policy(self.__arrangement), init=0, names='arrangement')
        
        self.add_verb2(1,'reset([],None)', callback=self.__reset)
        self.add_verb2(2,'play([],None)', callback=self.__play)
        self.add_verb2(3,'stop([],None)', callback=self.__stop)
        self.add_verb2(4,'undo([],None)', callback=self.__undo)
        self.add_verb2(5,'undo([un],None)', callback=self.__redo)
        
        self.live.set_window(self.top,self.height,self.left,self.width);
        
        
    def unload(self,destroy):
        self.live.shutdown();
        agent.Agent.unload(self,destroy) 

    def __reset(self,subj):
        self.live.refresh()
        
    def __play(self,subj):
        self.live.play()

    def __undo(self,subj):
        self.live.undo()

    def __redo(self,subj):
        self.live.redo()

    def __stop(self,subj):
        self[4].set_value(0)
        self.live.stop()

    def __arrangement(self,value):
        self[4].set_value(value)
        self.live.play_scene(value);
        return True
        
    def __top(self,value):
        if value > 0 :
            self.top=value;
            self[3][1].set_value(value)
            self.live.set_window(self.top,self.height,self.left,self.width);
            return True
        return False
    
    def __height(self,value):
        if value > 0 :
            self.height=value;
            self[3][2].set_value(value)
            self.live.set_window(self.top,self.height,self.left,self.width);
            return True
        return False
    
    def __left(self,value):
        if value > 0 :
            self.left=value;
            self[3][3].set_value(value)
            self.live.set_window(self.top,self.height,self.left,self.width);
            return True
        return False
    
    def __width(self,value):
        if value > 0 :
            self.width=value;
            self[3][4].set_value(value)
            self.live.set_window(self.top,self.height,self.left,self.width);
            return True
        return False

#
# Define Agent as this agents top level class
#
agent.main(Agent)
