
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

import picross
import piw
from pi import agent,atom,const,domain,bundles,resource,action,riff,files,node,upgrade
from . import clicker_version as version,loop_native
import os

class WavSample:
    def read(self,c):
        return c.read_external(self.__external)

    def __external(self,file,len):
        return file.read(len)

wav_reader = riff.Root('WAVE', riff.List(**{ 'fmt ': riff.Struct('<hHLLHH'), 'data': WavSample() }))

def fgetsamples(filename):
    print 'loading samples from ',filename
    f = open(filename,'rb',0)
    r = wav_reader.read(f)
    print 'sample rate is',r['fmt '][2]
    return loop_native.canonicalise_samples(r['data'],float(r['fmt '][2]))

def rgetsamples(res):
    print 'loading samples from ',res
    from cStringIO import StringIO
    r = files.PkgResourceFile(res)
    r2 = wav_reader.read(StringIO(r.data(0,r.size())))
    print 'sample rate is',r2['fmt '][2]
    return loop_native.canonicalise_samples(r2['data'],float(r2['fmt '][2]))

def wav_resource(name):
    print 'loading wav resource',name
    uf = resource.user_resource_file('loop',name,version='')
    if os.path.isfile(uf):
        return fgetsamples(uf)
    return rgetsamples('plg_loop/%s'%name)

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self,signature=version,names='clicker',container=3,ordinal=ordinal)
        self.domain = piw.clockdomain_ctl()
        accent = wav_resource('accent.wav')
        beat = wav_resource('beat.wav')

        self[1] = bundles.Output(1,True,names='audio output')
        self.output = bundles.Splitter(self.domain, self[1])
        self.clicker = loop_native.clicker(self.output.cookie(),self.domain,accent,beat)
        self.input = bundles.ScalarInput(self.clicker.cookie(), self.domain, signals=(1,2))

        self[2] = atom.Atom(names='inputs')
        self[2][1] = atom.Atom(domain=domain.Aniso(),policy=self.input.policy(1,False),names='running input')
        self[2][2] = atom.Atom(domain=domain.Aniso(),policy=self.input.policy(2,False),names='bar beat input')

        self.add_verb2(1,'play([],None)',self.__play,status_action=self.__status)
        self.add_verb2(2,'play([un],None)',self.__unplay,status_action=self.__status)
        self.add_verb2(3,'play([toggle],None)',self.__toggle,status_action=self.__status)
        
        self[4]=bundles.Output(1,False,names='status output')
        self.light_output=bundles.Splitter(self.domain,self[4])
        self.lights=piw.lightsource(piw.change_nb(),0,self.light_output.cookie())
        self.lights.set_size(1)
        self.lights.set_status(1,const.status_inactive)
        self.__playstate = node.Server(value=piw.makebool(True,0),change=self.__playchanged)
        self.set_private(self.__playstate)
        self.__playing=False

    def __status(self,subj):
        return 'dsc(~(s)"#4",None)'

    def __play(self,subj):
        self.clicker.play(True)
        self.__setup_playstate(True)
        self.lights.set_status(1,const.status_active)
        return action.nosync_return()

    def __unplay(self,subj):
        self.clicker.play(False)
        self.__setup_playstate(False)
        self.lights.set_status(1,const.status_inactive)
        return action.nosync_return()

    def __toggle(self,subj):
        print 'clicker __toogle: playing=',self.__playing
        if self.__playing:
            return self.__unplay(subj)
        else:
            return self.__play(subj)

    def __setup_playstate(self,playing):
        self.__playstate.set_data(piw.makebool(playing,0))
        self.__playing=playing

    def __playchanged(self,d):
        if d.is_bool():
            self.__playstate.set_data(d)
            self.__playing=d.as_bool()
            self.clicker.play(d.as_bool())

agent.main(Agent)
