
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

from pi import atom,bundles,domain

def channel_name(c,n):
    if c==2: return ('left','right')[n-1]
    return ''

class AudioOutput(atom.Atom):

    def __init__(self,splitter,base,channels,name='audio output',**kwds):
        atom.Atom.__init__(self,dynlist=True,names='outputs',**kwds)
        self.__base = base
        self.__channels = 0
        self.__splitter = splitter
        self.__name = name
        self.set_channels(channels)

    def cookie(self):
        return self.__splitter.cookie()

    def set_channels(self,channels):
        c = self.__channels

        if channels == c:
            return

        while c < channels:
            o = bundles.Output(self.__base+c,True,names=self.__name,ordinal=1+c)
            self.__splitter.add_output(o)
            self[1+c] = o
            c = c+1

        while c > channels:
            o = self[c]
            del self[c]
            self.__splitter.remove_output(o)
            c = c-1

        self.__channels = c
        for i in range(1,c+1):
            self[i].set_ordinal(i)
            self[i].set_names(channel_name(c,i)+' '+self.__name)

class AudioInput(atom.Atom):
    def __init__(self,input,base,channels,name='audio input',**kwds):
        atom.Atom.__init__(self,dynlist=True,**kwds)
        self.__base = base
        self.__channels = 0
        self.__input = input
        self.__name = name
        self.set_channels(channels)

    def set_channels(self,channels):
        c = self.__channels

        if channels == c:
            return

        while c < channels:
            o = atom.Atom(domain=domain.BoundedFloat(-1,1), policy=self.__get_policy(c),protocols='nostage',names=self.__name,ordinal=1+c)
            self[1+c] = o
            c = c+1

        while c > channels:
            o = self[c]
            del self[c]
            c = c-1

        self.__channels = c
        for i in range(1,c+1):
            self[i].set_ordinal(i)
            self[i].set_names(channel_name(c,i)+' '+self.__name)

    def __get_policy(self,c):
        p = self.__input.vector_policy if isinstance(self.__input,bundles.VectorInput) else self.__input.policy
        return p(self.__base+c,True)

class AudioChannels(atom.Atom):
    def __init__(self,*clients):
        self.__clients = clients
        atom.Atom.__init__(self,domain=domain.BoundedInt(0,128), names='channel count', init=1, policy=atom.default_policy(self.__set_channels))
        self.__set_channels(1)

    def set_channels(self,c):
        self.__set_channels(c)
        self.set_value(c)

    def __set_channels(self,c):
        for client in self.__clients:
            client.set_channels(c)
