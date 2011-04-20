
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
import loop_native
from pi import agent,atom,bundles,resource,audio,riff,files,domain
from plg_loop import audio_player_version as version
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
    uf = resource.user_resource_file('audio',name,version='')
    if os.path.isfile(uf):
        return fgetsamples(uf)
    return rgetsamples('audio/%s'%name)

class Agent(agent.Agent):
    def __init__(self,address, ordinal):
        agent.Agent.__init__(self,signature=version,names='audio player',ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()
        self.output = bundles.Splitter(self.domain)
        self[1] = audio.AudioOutput(self.output,1,2)
        file = wav_resource('output.wav')
        self.xplayer = loop_native.xplayer(self.output.cookie(),self.domain,file)
        self.xplayer.play(True)

        self[2] = atom.Atom(domain=domain.BoundedFloat(0,1),names='gain input',init=0.25,policy=atom.default_policy(self.__setgain))

    def __setgain(self,g):
        self.xplayer.set_gain(g)
        return True

agent.main(Agent)

