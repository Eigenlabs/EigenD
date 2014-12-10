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

#include "TrunkDrawing.h"

void drawTrunkBody(Graphics& g, float zoomFactor, int length, int thickness, int orientation, int x, int y,bool mouseOver)
{
    if (orientation==0)
    {
        drawHorizontalTrunkBody(g,zoomFactor,length,thickness, x, y,mouseOver);
    }
    else
    {
        drawVerticalTrunkBody(g,zoomFactor,length,thickness,  x, y,mouseOver);
    }

}

void drawHorizontalTrunkBody(Graphics& g, float zoomFactor, int length, int thickness, int x, int y,bool mouseOver)
{
    drawTrunkBody(g,zoomFactor,length,thickness,x,y,mouseOver,false);
}

void drawVerticalTrunkBody(Graphics& g, float zoomFactor, int length, int thickness, int x, int y,bool mouseOver)
{

    drawTrunkBody(g,zoomFactor,length,thickness,x,y,mouseOver,true);
}

void drawTrunkBody(Graphics& g, float zoomFactor, int length, int thickness, int x, int y,bool mouseOver,bool vertical)
{
    Colour col1;
    Colour col2;

    if (length>(36))
    {
        if(mouseOver)
        {
            col1=Colour(mouseOverTrunkColour1);
            col2=Colour(mouseOverTrunkColour2);
        }
        else
        {
            col1=Colour(trunkColour1);
            col2=Colour(trunkColour2);
        }

        Path p=getTrueTrunkOutline(length,thickness);
        p.applyTransform(AffineTransform::scale(zoomFactor, zoomFactor));

        if (vertical)
        {
            //rotate the path
            float x0=10.0f*zoomFactor;
            float y0=(thickness-10.0f)*zoomFactor;
            p.applyTransform(AffineTransform::rotation(3.1415f*0.5f, x0,y0));
            p.applyTransform(AffineTransform::translation(0.0f, -1.0f*(thickness-20.0f)*zoomFactor));
            // shade in other direction
            g.setGradientFill(ColourGradient(col1,x*zoomFactor,0,col2,(x+thickness)*zoomFactor,0,false));
        }
        else
        {
            int y0=thickness-10.0f;
            g.setGradientFill(ColourGradient(col1,0,y*zoomFactor,col2,0,(y+y0)*zoomFactor,false));
        }

        p.applyTransform(AffineTransform::translation(x*zoomFactor, y*zoomFactor));
        g.fillPath(p);
        g.setColour (Colour(0xff817e7e));
        float lineThickness=2.0f*zoomFactor;
        g.strokePath(p,PathStrokeType(lineThickness));
    }
}

Path getTrunkOutline(float zoomFactor, int length, int thickness)
{
    Path p;
    p.clear();
    int bx=10.0f*zoomFactor;
    int by=10.0f*zoomFactor;

    int x0=bx;
    int x1=length-bx;
    int y0=thickness-by;
    int y1=by;
    float mid=y1+0.5*(y0-y1);

    float dx=16.0f*zoomFactor;
    float sectionLength=5.0f*dx;
    p.startNewSubPath(x0,y0);
    p.quadraticTo(x0+8*zoomFactor,mid, x0,y1);
    p.lineTo(x0+dx,y1);

    int sectionCount=0;
    int xStart=x0+dx;
    int x=xStart;

    while(x<=(x1-sectionLength))
    {
        p.lineTo(x,mid);
        x=x+dx;
        p.quadraticTo(x-0.5*dx,mid+(8*zoomFactor),x,mid);
        p.lineTo(x,y1);
        sectionCount++;
        x=xStart+sectionCount*sectionLength;
        p.lineTo(x,y1);
    }

    if (x<=(x1-(2.0f*dx)))
    {
        p.lineTo(x,mid);
        x=x+dx;
        p.quadraticTo(x-0.5*dx,mid+(8*zoomFactor),x,mid);
        p.lineTo(x,y1);
    }

    p.lineTo(x1,y1);
    p.quadraticTo(x1-8*zoomFactor,mid,x1,y0);
    p.lineTo(x,y0);
    x=sectionCount*sectionLength;
    if (x<x0)
    {
        x=x0;
    }
    p.lineTo(x,y0);

    while (sectionCount>0)
    {
        p.lineTo(x,mid);
        x=x-dx;
        p.quadraticTo(x+0.5*dx,mid-(8*zoomFactor),x,mid);
        p.lineTo(x,y0);
        sectionCount--;
        x=sectionCount*sectionLength;
        if (x<x0)
        {
            x=x0;
        }
        p.lineTo(x,y0);
    }

    p.closeSubPath();
    return p;
}

Path getTrueTrunkOutline(int length, int thickness)
{
    return getTrunkOutline(1.0f,length,thickness);
}


