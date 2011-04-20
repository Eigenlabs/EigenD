
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
from pi import riff
from pi.riff import List,Struct,StructList,String,Root
import math
import copy
import struct

def read_samples_indirect(name,pos,len):
    return piw.create_samplearray(name,pos,len)

class Sample:
    def read(self,c):
        return c.read_indirect(read_samples_indirect)

GEN_STARTADDROFS=0
GEN_ENDADDROFS=1
GEN_STARTLOOPADDROFS=2
GEN_ENDLOOPADDROFS=3
GEN_STARTADDRCOARSEOFS=4
GEN_ENDADDRCOARSEOFS=12
GEN_PAN=17
GEN_INSTRUMENT=41
GEN_KEYRANGE=43
GEN_VELRANGE=44
GEN_INITIALATTENUATION=48
GEN_STARTLOOPADDRCOARSEOFS=45
GEN_ENDLOOPADDRCOARSEOFS=50
GEN_COARSETUNE=51
GEN_FINETUNE=52
GEN_SAMPLEID=53
GEN_SAMPLEMODE=54
GEN_OVERRIDEROOTKEY=58
GEN_AHDSR_DELAY=33
GEN_AHDSR_ATTACK=34
GEN_AHDSR_HOLD=35
GEN_AHDSR_DECAY=36
GEN_AHDSR_SUSTAIN=37
GEN_AHDSR_RELEASE=38

gen_defaults = {
    GEN_AHDSR_DELAY: -12000.0,
    GEN_AHDSR_ATTACK: -12000.0,
    GEN_AHDSR_HOLD: -12000.0,
    GEN_AHDSR_SUSTAIN: 0.0,
    GEN_AHDSR_RELEASE: -12000.0,
    GEN_AHDSR_DECAY: -12000.0,
    GEN_COARSETUNE: 0.0,
    GEN_FINETUNE: 0.0,
    GEN_KEYRANGE: (0,127),
    GEN_VELRANGE: (0,127),
}

gen_preset_ignore = ( GEN_KEYRANGE, GEN_VELRANGE )

class GenList:
    def read1(self,r):
        id = struct.unpack('<H',r[0:2])[0]
        if id in (GEN_KEYRANGE,GEN_VELRANGE):
            return (id,struct.unpack('BB',r[2:4]))
        else:
            return (id,struct.unpack('<h',r[2:4])[0])

    def read(self,c):
        s = []
        while c.remain>=4:
            s.append(self.read1(c.read(4)))
        return s

SF2 = Root('sfbk', List(
            INFO=List(
                ifil=Struct('<HH'), isng=String(), INAM=String(), irom=String(),
                iver=Struct('<HH'), ICRD=String(), IENG=String(), IPRD=String(),
                ICOP=String(), ICMT=String(65536), ISFT=String()),
            sdta=List(smpl=Sample()),
            pdta=List(
                phdr=StructList('<20sHHHLLL'), pbag=StructList('<HH'), pmod=StructList('<HHhHH'), pgen=GenList(),
                inst=StructList('<20sH'), ibag=StructList('<HH'), imod=StructList('<HHhHH'), igen=GenList(),
                shdr=StructList('<20sLLLLLBbHH'))))


def mtof(m,transpose):
    m = float(m)-float(transpose)
    return 440.0*math.pow(2.0,(m-69.0)/12.0)

def mtov(m):
    return float(m)/127.0

def etot(m):
    m = pow(2.0,float(m)/1200.0)
    if m<0.01: m=0
    return m

def etos(m):
    if m <= 0: return 1.0
    if m >= 1000: return 0.0
    return 1.0-(float(m)/1000.0)

def etop(m):
    return float(m)/500.0

class ZoneBuilder:
    def __init__(self,bag,gen,mod,index,base=None,add=None):
        self.gen = {}

        if base: 
            self.gen.update(base.gen)

        gs,ms = bag[index]
        ge,me = bag[index+1]

        for g in range(gs,ge): self.__addgen(gen[g])
        for m in range(ms,me): self.__addmod(mod[m])

        if add:
            for k,v in add.gen.iteritems():
                if k in gen_preset_ignore:
                    continue

                o = self.genget(k)

                if type(v)==tuple:
                    #self.gen[k] = (max(o[0],v[0]),min(o[1],v[1]))
                    self.gen[k] = v
                else:
                    self.gen[k] = o+v

    def __addgen(self,g):
        self.gen[g[0]] = g[1]

    def __addmod(self,m):
        pass

    def __adjustpos(self,val,fg,cg):
        return val+self.genget(fg)+(32768*self.genget(cg))

    def genget(self,k):
        return self.gen.get(k,gen_defaults.get(k,0))

    def zone(self,smpl,shdr,transpose):
        kr = self.genget(GEN_KEYRANGE)
        vr = self.genget(GEN_VELRANGE)

        de = etot(self.genget(GEN_AHDSR_DELAY))
        a = etot(self.genget(GEN_AHDSR_ATTACK))
        h = etot(self.genget(GEN_AHDSR_HOLD))
        dc = etot(self.genget(GEN_AHDSR_DECAY))
        sus = etos(self.genget(GEN_AHDSR_SUSTAIN))
        r = etot(self.genget(GEN_AHDSR_RELEASE))
        p = etop(self.genget(GEN_PAN))

        n,s,e,ls,le,sr,op,_,_,_ = shdr[self.gen[GEN_SAMPLEID]]

        rk =  float(self.gen.get(GEN_OVERRIDEROOTKEY,op))
        rk -= float(self.genget(GEN_COARSETUNE))
        rk -= (float(self.genget(GEN_FINETUNE))/100.0)

        rf = mtof(rk,transpose)

        looping = False
        if self.gen.has_key(GEN_SAMPLEMODE):
            if self.gen[GEN_SAMPLEMODE] != 0:
                looping = True

        start = self.__adjustpos(s,GEN_STARTADDROFS,GEN_STARTADDRCOARSEOFS)
        end   = self.__adjustpos(e,GEN_ENDADDROFS,GEN_ENDADDRCOARSEOFS)
        
        if looping:
            loopstart = self.__adjustpos(ls,GEN_STARTLOOPADDROFS,GEN_STARTLOOPADDRCOARSEOFS)
            loopend   = self.__adjustpos(le,GEN_ENDLOOPADDROFS,GEN_ENDLOOPADDRCOARSEOFS)
        else:
            loopstart = 0
            loopend = 0

        attcb = float(self.gen.get(GEN_INITIALATTENUATION,0))
        att = math.pow(10.0,-attcb/200.0)
        smpl = piw.create_sample(smpl,start,end,loopstart,loopend,sr,rf,att)
        zz = piw.create_zone(mtof(float(kr[0])-0.5,transpose), mtof(float(kr[1])+0.5,transpose), mtov(float(vr[0])-0.5), mtov(float(vr[1])+0.5),de,a,h,dc,sus,r,p,smpl)
        return zz

    def __str__(self):
        return str(self.gen)

def load_soundfont(file,bk,pre,transpose):
    print 'loading bank',bk,'preset',pre,'from',file
    f = open(file,'rb',0)
    sf = SF2.read(f,name=file)
    f.close()

    pbs = None
    pbe = None

    for (n,p,b,i,l,g,m) in sf['pdta']['phdr']:

        if pbs is not None:
            pbe = i
            break

        if p==pre and b==bk:
            pbs = i

    if pbs is None or pbe is None:
        raise RuntimeError('preset %d bank %d not found in soundfont %s' % (pre,bk,file))

    p = piw.create_preset()

    gpzb = None
    gizb = None

    for pi in range(pbs,pbe):
        pzb = ZoneBuilder(sf['pdta']['pbag'],sf['pdta']['pgen'],sf['pdta']['pmod'],pi,base=gpzb)
        inst = pzb.gen.get(GEN_INSTRUMENT)
        if inst is not None:
            for ii in range(sf['pdta']['inst'][inst][1],sf['pdta']['inst'][inst+1][1]):
                izb = ZoneBuilder(sf['pdta']['ibag'],sf['pdta']['igen'],sf['pdta']['imod'],ii,base=gizb,add=pzb)
                if izb.gen.has_key(GEN_SAMPLEID):
                    p.add_zone(izb.zone(sf['sdta']['smpl'],sf['pdta']['shdr'],transpose))
                else:
                    if gizb is None:
                        gizb = izb
        else:
            if gpzb is None:
                gpzb = pzb

    return p

SF2info = Root('sfbk', List(pdta=List(phdr=StructList('<20sHH14x'))))

def __trim(s):
    if s.count('\0'):
        return s[:s.index('\0')]
    return s

def sf_info(file):
    file = open(file,'rb',0)
    data = SF2info.read(file)
    file.close()
    for n,p,b in data['pdta']['phdr'][:-1]:
        yield __trim(n),p,b


