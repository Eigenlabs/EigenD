/*
 Copyright 2012-2014 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#include "juce.h"
#include <iostream>
#include "math.h"

//#define MAXERR 1e-10
//#define MAXIT 100

//#define MAXERR 1e-8
//#define MAXIT 50

#define MAXERR 1e-5
#define MAXIT 30


//#define TV((upper+lower)/2)

Path getQuadraticBezierPath(int x1, int y1, int x2, int y2);
Path getCatenaryPath(int x1,int y1,int x2,int y2,float lengthFactor, int height,float thickness,int quality);
Path getLinearPath(int x1,int y1,int x2,int y2,int height,float thickness,int quality);

double atanh(double x);
double Calc_D(double a, double L, double h, double sgn);
double Solve_h(double a, double L, double d);

double calc_y(double x, double u, double xx1, double k );
double getSign(double a, double L,double d);
double getL1(double h, double L, double a, double d);
double getu(double h, double L1);
double getX1(double u, double L1);
double getk(double h, double h1,double u);
int getNumSamples(double d, int quality);
