
/*
 Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com

 This file is part of EigenD.

 EigenD is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 EigenD is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __LOOP_MOD__
#define __LOOP_MOD__

#include <math.h>

namespace loop
{
    template<class X> X mod(X x, X y) { return x % y; }
    template<> inline double mod(double x, double y) { return fmod(x,y); }
    template<> inline float mod(float x, float y) { return fmodf(x,y); }
    template<class X> X distance(X lo, X hi, X m) { return mod(m + hi - lo, m); }
    template<class X> X back(X x, X back, X m)  { return distance(back, x, m); }
    template<class X> X fwd(X x, X fwd, X m) { return mod(x + fwd, m); }
    template<class X> bool brackets(X lo, X hi, X x) { if(lo==hi) return false; if(lo<hi) return lo<=x && x<=hi; return x>lo || x<hi; }
}

#endif
