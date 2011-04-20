
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


from numpy import *
from numpy.fft import fft,ifft
from scipy.interpolate import UnivariateSpline
from scipy.stats import linregress
import math

def sinctable(p,os,nz,f):
    po = int(p*os)
    assert po%2==0
    nz = float(nz)/f
    w = int(ceil(os*2*nz))
    if w%2==0: w+=1
    phase = linspace(-nz,nz,w)
    sm = sinc(f*phase)
    sm = sm*blackman(w)
    pl = int(po/2-w/2)
    pad1 = [0.0]*pl
    pad2 = [0.0]*(pl-1)
    ret = array(pad1+list(sm)+pad2)
    return ret,w

def rcwin(n):
    l = [1.0]
    l.extend((n/2-1)*[2.0])
    if n%2==0:
        l.append(1.0)
    else:
        l.append(2.0)
        l.append(1.0)
    l.extend((n/2-1)*[0.0])
    return array(l)

def rceps(v):
    y = real(ifft(log(abs(fft(v)))))
    n = len(y)
    wy = rcwin(n)*y
    return y,real(ifft(exp(fft(wy))))

period = float(96000/20)
oversampling = 128
zc = 64
f = 0.9
delay = 4

def shift(v,s):
    ret = []
    for i in range(0,len(v)):
        si = float(i)-s
        if si<1:
            ret.append(0.0)
            continue

        fsi = floor(si)
        csi = ceil(si)
        if csi==fsi: csi += 1
        ax = [fsi-1,fsi,csi,csi+1]
        ay = [v[x] for x in ax]
        us = UnivariateSpline(ax,ay)
        ret.append(us(si))
    return ret

if __name__ == '__main__':
    s,w = sinctable(period,oversampling,zc,f)
    fo = open('sinc.out','w')
    for x in s: fo.write('%.18f\n'%x)
    fo.close()

    rc = rceps(s)[1]
    fo = open('rceps.out','w')
    for x in rc: fo.write('%.18f\n'%x)
    fo.close()

    blep = add.accumulate(rc)
    blep = blep/blep[-1]
    fo = open('blep.out','w')
    for x in blep: fo.write('%.18f\n'%x)
    fo.close()

    bl = len(blep)/4
    ramp = add.accumulate(blep)[bl:bl*2]
    xramp = arange(bl,bl*2)
    g,i,r,p,e = linregress(xramp,ramp)
    print 'intercept is',i

    osd = delay*oversampling
    print 'shift is',osd+i
    sblep = shift(blep,osd+i)
    fo = open('sblep.out','w')
    for x in sblep: fo.write('%.18f\n'%x)
    fo.close()

    # 18176 is a zero crossing and divides the oversampling
    # exactly (142*128)
    tblep = array(list(sblep[:18176]))

    fo = open('tblep.out','w')
    for x in tblep: fo.write('%.18f\n'%x)
    fo.close()

    sub = tblep+array([0.0]*osd+[-1.0]*(len(tblep)-osd))
    fo = open('sub.out','w')
    for x in sub: fo.write('%.18f\n'%x)
    fo.close()

    deltas = []
    for i in range(0,len(tblep)-1):
        x = tblep[i]
        nxt = tblep[i+1]
        deltas.append(nxt-x)
    deltas.append(0.0)
    fo = open('deltas.out','w')
    for x in deltas: fo.write('%.18f\n'%x)
    fo.close()

    ss = len(tblep)-osd
    r = linspace(0.0,-(float(ss-1)/float(oversampling)),ss)
    cusp = array([0.0]*osd+list(r))
    fo = open('cusp.out','w')
    for x in cusp: fo.write('%.18f\n'%x)
    fo.close()

    iblep = add.accumulate(tblep)/float(oversampling)
    fo = open('iblep.out','w')
    for x in iblep: fo.write('%.18f\n'%x)
    fo.close()

    isub = iblep+cusp
    fo = open('isub.out','w')
    for x in isub: fo.write('%.18f\n'%x)
    fo.close()

    deltas = []
    for i in range(0,len(isub)-1):
        x = isub[i]
        nxt = isub[i+1]
        deltas.append(nxt-x)
    deltas.append(0.0)
    fo = open('ideltas.out','w')
    for x in deltas: fo.write('%.18f\n'%x)
    fo.close()

