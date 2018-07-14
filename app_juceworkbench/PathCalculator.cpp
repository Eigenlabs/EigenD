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

#include "PathCalculator.h"

Path getQuadraticBezierPath(int x1, int y1,int x2, int y2)
{
    Path p;
    float dx=(x2-x1);
    float dy=(y2-y1);
    float cx=x1+(0.5*dx)-(0.25*dy);
    float cy=y1+(0.5*dy)+(0.25*dx);

    Line<float> l(x1,y1,x1+0.01,y1+0.01);
    p.addLineSegment(l,2.0f);
    p.quadraticTo(cx,cy,x2,y2);
    return p;
}

Path getLinearPath(int x1, int y1, int x2, int y2, int height,float thickness, int quality)
{
  // convert y coordinate to increasing upwards
    y1=height-y1;
    y2=height-y2;

    double a=y2-y1;  //difference in height
    int dd=x2-x1;  //distance between two poles
    if (dd==0)
    {
        dd=1;
    }
    double d=abs(dd);  //absolute distance between two poles
 
    double L=sqrt((a*a) +(d*d));//Length of wire (straight line distance +30%)
    double m=a/(dd);
    // XX sample the function at a range of x and add to path
    // convert back to screen coordinates
    Path p;
    double x=0.0;
    double y=0.0;
    double oldx=0.0;
    double oldy=double(y1);

    int count=0;
    int numSamples;
    numSamples=getNumSamples(L,quality);
//    if(quality==2)
//    {
//        numSamples=getDraftNumSamples(L);
//    }
//    else
//    {
//        numSamples=getNumSamples(L);
//    }
    while (count<numSamples)
    {
        x=x+(dd/float(numSamples));
        y=(m*x)+y1;
    //    printf("x= %3.1f y=%3.1f m=%10.5f \n\r ",float(x),float(y), float(m));
        Line<float> l(float(x1+oldx),float(height-oldy),float(x1+x),float(height-y));
        p.addLineSegment(l,thickness);
        oldx=x;
        oldy=y;
        ++count;
    }

//    // XXX simplification - but requires changes in calculation of gridsquares for wire detection
//    Path p;
////    printf("x1= %3.1f y1=%3.1f x2= %3.1f y2=%3.1f  \n\r ",float(x1),float(y1),float(x2),float(y2));
//    Line<float> l(x1,y1,x2,y2);
//    p.addLineSegment(l,thickness);

    return p;

}

Path getCatenaryPath(int x1,int y1,int x2,int y2, float lengthFactor,int height,float thickness,int quality)
{
    
    // convert y coordinate to increasing upwards
    y1=height-y1;
    y2=height-y2;
    double a=y2-y1;  //difference in height
    //printf("Difference in height a= %3.1f\n\r",a);
    if (a==0.0)
    {
        a=0.001;
    }

    int dd=x2-x1;  //distance between two poles
    if (dd==0)
    {
        dd=1;
    }
    double d=abs(dd);  //absolute distance between two poles
    //printf("absolute refined distance between poles d= %10.6f\n\r",d);

    double L=lengthFactor*sqrt((a*a) +(d*d));//Length of wire (straight line distance +30%)
//    double L=28.0;
//    printf("Length of wire L= %3.1f\n\r",L);
    double h=Solve_h(a,L,d);
    //printf("h= %3.1f\n\r",h);
    // calculate the curve
    double L1=getL1(h,L,a,d);
    //printf("L1= %3.1f\n\r",L1);
    double u = getu(h,L1); 
    //printf("u= %5.3f\n\r",u);
    double xx1=getX1(u,L1);
    //printf("xx1= %3.1f\n\r",xx1);
    double k=getk(h,y1,u);
    //printf("k= %3.1f\n\r",k);
    // create and return a path
    //double yy1=((cosh(0.0f-u*xx1))/u)+k;
    //double yy2=((cosh(u*double(x2-x1)-u*xx1))/u)+k;
    //printf("******at %3.1f yy1=%3.1f at %3.1f yy2=%3.1f\n\r",0.0f,yy1,double(x2-x1),yy2);

    // XX sample the function at a range of x and add to path
    // convert back to screen coordinates
    Path p;
    double x=0.0;
    double y=0.0;
    double oldx=0.0;
    double oldy=double(y1);

    int count=0;
    int numSamples;
    numSamples=getNumSamples(L,quality);
//    if(quality==2)
//    {
//        numSamples=getDraftNumSamples(L);
//    }
//    else
//    {
//        numSamples=getNumSamples(L);
//    }
    while (count<numSamples)
    {
        x=x+(d/float(numSamples));
        y=calc_y(x,u,xx1,k);
        //printf("x= %3.1f y=%3.1f \n\r",float(x),float(y));
        if (dd>0)
        {
            Line<float> l(float(x1+oldx),float(height-oldy),float(x1+x),float(height-y));
            p.addLineSegment(l,thickness);
        }
        else
        {
            Line<float> l(float(x1-oldx),float(height-oldy),float(x1-x),float(height-y));
            p.addLineSegment(l,thickness);
           
        }
        oldx=x;
        oldy=y;
        ++count;
    }
    return p;
}

int getNumSamples(double d,int quality)
{
    int n=0; 
    if(quality==1)
    {
        n =d/60;
        if(n<16)
        {
            n=16;
        }
    }
    else if (quality==2)
    {
        n =d/250;
        if(n<8)
        {
            n=8;
        }
    }
    return n;
}

double calc_y(double x, double u, double xx1, double k )
{
    return ((cosh(u*double(x)-u*xx1))/u)+k;
}

double atanh(double x)
{
    return 0.5* log((1+x)/(1-x));
}

double Calc_D(double a, double L, double h, double sgn)
{
    double q=2*sgn*sqrt(h*(a+h)*(L*L-a*a));
    return ((L*L-a*a)*(a+2*h)-L*q)/(a*a)*atanh(a*a/(L*(a+2*h)-q));
}

double getSign(double a, double L,double d)
{
    double s=((L*L-a*a)/(2*a)*log((L+a)/(L-a))>d) ?-1:1;
    return s;
}
double getL1(double h, double L, double a,double d)
{
    double sign=getSign(a,L,d);
    return -1.0f*((h*L)+sign*(sqrt(h*(a+h)*((L*L)-(a*a)))))/a;
}

double getu(double h, double L1)
{
    return (2.0f*h)/((L1*L1)-(h*h));
}

#ifndef PI_WINDOWS
#define pic_asinh(n) asinh(n)
#else
inline double pic_asinh(double n) { return log(n+sqrt(n*n+1)); }
#endif

double getX1(double u, double L1)
{
    return (pic_asinh(u*L1))/u;
}

double getk(double h, double h1,double u)
{
    return h1-h-(1.0f/u);
}

double Solve_h(double a, double L, double d)
{
    int n=1;
    double s=((L*L-a*a)/(2*a)*log((L+a)/(L-a))<d) ?-1:1;
    double lower=0, upper=(L-a)/2;

    while ((upper-lower)>MAXERR && (++n)<MAXIT)
        if(Calc_D(a,L,(upper+lower)/2,s)*s<d*s)upper=(upper+lower)/2;
        else lower=(upper+lower)/2;
        //printf("Found h=%3.1f after %d iterations.\n\r",(upper+lower)/2,n);
    return s*((upper+lower)/2);
}




