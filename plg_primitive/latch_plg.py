#
# Copyright 2011 Eigenlabs Ltd.  http://www.eigenlabs.com
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
from pi import agent, domain, bundles, atom
from plg_primitive import latch_version as version
from .primitive_plg_native import latch

#
# The Latch agent that uses the controller signal and a minimum
# threshold to determine whether the other signals can send through their
# data. As soon as the threshold is exceeded, the data of the other signals
# will pass through unchanged. However, when the controller signal drops
# below the minimum threshold, the last known value of the other signals is
# sent instead of their real values.
#
class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        #
        # Agents are constructed with the address and ordinal.  Each
        # agent will have a unique address and an ordinal unique for
        # that type of agent.
        #
        # We also pass in the automatically generated version to go into
        # the metadata.
        #
        agent.Agent.__init__(self, signature=version, names='latch', ordinal=ordinal)

        #
        # Create a clock domain.  A clock domain is a collection of clock
        # sinks all of which share common properties.  Domains can be isochronous
        # or anisochronous.  Currently, sinks in iso domains can be connected
        # only to other iso domains using the same source.
        #
        self.domain = piw.clockdomain_ctl()

        #
        # Make our domain anisochronous (aniso is used for everything except audio)
        #
        self.domain.set_source(piw.makestring('*',0))

		#
		# Create an atom to bundle the outputs also create sub-atoms that correspond to
		# each of the input streams.
        # An output is created on each sub-atom with a dedicated signal number and name.
        # Setting the second argument to false means that they're not isochronous data streams.
		#
        self[2] = atom.Atom(names="outputs")
        self[2][1] = bundles.Output(1, False, names='pressure output')
        self[2][2] = bundles.Output(2, False, names='roll output')
        self[2][3] = bundles.Output(3, False, names='yaw output')

        #
        # bundles.Splitter is a bundle component which demultiplexes the events onto separate
        # Outputs, 1 output per signal.  The bundle, which carries events containing signals,
        # is represented on the output as signals containing events.
		#
        self.output = bundles.Splitter(self.domain,*self[2].values())
        
        #
        # Create an instance of the native Latch implementation and pass it the clock domain
        # and the upstream cookie for the output data.
        #
        self.native = latch(self.domain, self.output.cookie())

        #
        # Create an input.  An input does the work of correlating the events on a 
        # number of signals, grouping them together.
        # 
        # A Vector input is a proper multiple channel input. This one is being
        # constructed to handle 4 signals, numbered 1,2,3 and 4. The events will
        # be sent to the output stage through the use of the native implementation's cookie.
        #
        self.key_input = bundles.VectorInput(self.native.cookie(), self.domain, signals=(1,2,3))

		#
        # Create a null atom just to group our inputs. We still name this atom so that it
        # shows up with a meaningful name through introspection by tools like Workbench.
        #
        self[1] = atom.Atom(names="inputs")

        #
        # Now create our inputs. The data policy for the atom is created by our vector input.
        # There are lots of corner cases when we invert our model from signal-contains-events
        # to event-contains-signals. In particular, this happens when events on individual
        # signals dont start and and stop together.
        #
        # Any input with a vector_policy will create or join an existing event.
        # merge_policy will never create events, just join onto existing events created by
        # a vector policy.
        #
        # We create 4 inputs, with vector policies routing them to signals 1,2,3 and 4 on
        # our bundle. These are all non-iso inputs.
        #
        self[1][1] = atom.Atom(domain=domain.BoundedFloat(0,1), policy=self.key_input.vector_policy(1,False), names='pressure input')
        self[1][2] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.key_input.vector_policy(2,False), names='roll input')
        self[1][3] = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.key_input.vector_policy(3,False), names='yaw input')

        #
        # Lastly, create atoms called 'minimum' and 'controller' to store values for those.
        # These will appear in Stage and can be set using Belcanto. When they changes, the 
        # methods __minimum and __controller will be called with a Python numeric argument
        # (because its a numeric domain).
        #
        self[3] = atom.Atom(domain=domain.BoundedFloat(0,1), init=0.5, policy=atom.default_policy(self.__minimum), names='minimum')
        self[4] = atom.Atom(domain=domain.BoundedInt(1,3), init=1, policy=atom.default_policy(self.__controller), names='controller')

	#
	# This method will be called when the 'minimum' atom is changed and
	# is connection up through the policy.
	#
    def __minimum(self,m):
        self[3].set_value(m)
        self.native.set_minimum(m)
        return True

	#
	# This method will be called when the 'controller' atom is changed and
	# is connection up through the policy.
	#
    def __controller(self,c):
        self[4].set_value(c)
        self.native.set_controller(c)
        return True

agent.main(Agent)
