
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

import random
import wx
from pigui import colours,fonts,mathutils

DEFAULT_W=100
DEFAULT_H=50

def getRedColour():
    r=random.randrange(0,256,16) 
    g=random.randrange(0,64,16) 
    return wx.Colour(r,g,0)

def getColour():
    r=random.randrange(0,128,16) 
    g=random.randrange(0,256,16) 
    b=random.randrange(0,256,16) 
    return wx.Colour(r,g,b)

def getAnyColour():
    r=random.randrange(0,256,16) 
    g=random.randrange(0,256,16) 
    b=random.randrange(0,256,16) 
    return wx.Colour(r,g,b)

def setPenColour(dc,colour):
    pen=dc.GetPen()
    pen.SetColour(colour)
    dc.SetPen(pen)

def getPenColour(dc):
    pen=dc.GetPen()
    return pen.GetColour()

def setPenWidth(dc,width):
    pen=dc.GetPen()
    pen.SetWidth(width)
    dc.SetPen(pen)

def setBrushColour(dc,colour):
    brush=dc.GetBrush()
    brush.SetColour(colour)
    dc.SetBrush(brush)

def getBackgroundColour(dc):
    brush=dc.GetBackground()
    return brush.GetColour()

def getDisplayText(text,allowedWidth,dc):
    displaytext=text
    extent=dc.GetTextExtent(displaytext)
    count=-1 
    while extent[0]>allowedWidth: 
        displaytext=text[:count]+'...'
        extent=dc.GetTextExtent(displaytext)
        count=count-1
        if count < -1*len(text):
            return ''
    return displaytext         


def drawWindowHeader(parent,title,dc,blank=False):
    drawHeader(parent.GetSize(),title,dc,blank=blank)  

def drawHeader(size,title,dc,position=(0,0),blank=False):
    print 'drawHeader',title 
    if blank:
        viewBackground=colours.defaultViewBackground
        dc.SetPen(wx.Pen(viewBackground))
        dc.SetBrush(wx.Brush(viewBackground))
        dc.DrawRectangle(position[0],position[1],size[0],20)
    else:
        #position=(size[0]*position[0],size[1]*position[1])
        titleBarBackground=wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        dc.SetPen(wx.Pen(titleBarBackground))
        dc.SetBrush(wx.Brush(titleBarBackground))
        dc.DrawRectangle(position[0],position[1],size[0],20)
        dc.GradientFillLinear(wx.Rect(position[0],position[1],size[0],20),colours.titleBarGradient1,colours.titleBarGradient2,wx.SOUTH)
        dc.SetPen(wx.Pen(colours.titleBarEdge))
        dc.DrawLine(position[0],position[1],size[0],position[1])
        dc.DrawLine(position[0],position[1]+20,position[0]+size[0],position[1]+20)
        
        dc.SetBrush(wx.Brush('WHITE'))
        dc.SetPen(wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_CAPTIONTEXT)))
        pts,weight=fonts.setTitleBarFont(dc)
        xpos=0.5*(size[0]-dc.GetTextExtent(title)[0])
        dc.DrawText(title,xpos,position[1]+3)
        fonts.resetFont(dc,pts,weight)


class ConnectionPoint(mathutils.point):
    def __init__(self,x,y,side=True,id=0):
        mathutils.point.__init__(self,x,y)
        self.free=True 
        self.side=side
        self.id=id

    def setUsed(self):
        self.free=False

    def setFree(self):
        self.free=True

    def isFree(self):
        return self.free

    def isSide(self):
        return self.side

class PrincipalConnectionPoint(ConnectionPoint):
    def __init__(self,x,y,side=True,name=''):
        ConnectionPoint.__init__(self,x,y,side)
        self.points=[ConnectionPoint(x,y,side)]
        self.name=name  

    def addPoint(self,point):
        self.points.append(point)

    def isFree(self):
        for p in self.points:
            if p.isFree():
                return True
        return False    

    def setFree(self):
        for p in self.points:
            p.setFree()

    def getFreePoint(self):
        for p in self.points:
            if p.isFree():
                p.setUsed()
                return p
        return self.points[0]

class rectangle:
    def __init__(self,x,y,w=DEFAULT_W,h=DEFAULT_H,edgeColour=colours.defaultRectangleEdge,fillColour=colours.defaultRectangleFill,edgeThickness=1):
        self.x=x
        self.y=y
        self.w=w
        self.h=h
        self.edgeColour=edgeColour
        self.fillColour=fillColour
        self.setupConnectionPoints()
        self.edgeThickness=edgeThickness
        self.gradientColours=None
    
    def setGradientColours(self,gradientColours):
        self.gradientColours=gradientColours

    def deltaMove(self,dx,dy):
        self.x=self.x+dx
        self.y =self.y +dy

    def draw(self,dc,xOffset=0,yOffset=0):

        pen=dc.GetPen()
        pen.SetColour(self.edgeColour)
        pen.SetWidth(self.edgeThickness)
        dc.SetPen(pen)
       
        brush=dc.GetBrush()
        dc.SetBrush(wx.Brush(self.fillColour)) # pale yellow
        dc.DrawPolygon(self.getPoints(),self.x-xOffset,self.y-yOffset)

        if self.gradientColours:
            dc.GradientFillLinear(wx.Rect(self.x-xOffset,self.y-yOffset,self.w,self.h),self.gradientColours[0],self.gradientColours[1],wx.SOUTH)
        dc.SetBrush(brush)
        pen=dc.GetPen()
        pen.SetColour('BLACK')
        pen.SetWidth(1)
        dc.SetPen(pen)

    def getPoints(self):
        points=[]
        points.append(wx.Point(0,0))
        points.append(wx.Point(self.w,0))
        points.append(wx.Point(self.w,self.h))
        points.append(wx.Point(0,self.h))
        return points

    def getLines(self):
        rect=[]
        rect.append((self.x,self.y,self.x+self.w,self.y))
        rect.append((self.x,self.y,self.x,self.y+self.h))
        rect.append((self.x,self.y+self.h,self.x+self.w,self.y+self.h))
        rect.append((self.x+self.w,self.y+self.h,self.x+self.w,self.y))
        
        return rect
 
    def setFillColour(self,col):
        self.fillColour=col
    
    def setEdgeThickness(self,thickness):
        self.edgeThickness=thickness

    def getVertices(self):
        vertices=[]
        vertices.append(mathutils.point(self.x,self.y))
        vertices.append(mathutils.point(self.x+self.w,self.y))
        vertices.append(mathutils.point(self.x,self.y+self.h))
        vertices.append(mathutils.point(self.x+self.w,self.y+self.h))
        return vertices
    
    def getMidPoints(self):
        midPoints=[]
        midPoints.append(mathutils.point(self.x,self.y+(self.h/2)))
        midPoints.append(mathutils.point(self.x+self.w,self.y+(self.h/2)))
        midPoints.append(mathutils.point((self.x+self.w/2),self.y))
        midPoints.append(mathutils.point((self.x+self.w/2),self.y+self.h))
        return midPoints

    def getConnectionPoints(self):
        return self.cplist

    def setupConnectionPoints(self):
        self.cplist=[]
        p=0.5
        self.cplist.append(PrincipalConnectionPoint(self.x,self.y+(self.h*p),name='left'))
        self.cplist.append(PrincipalConnectionPoint(self.x+self.w,self.y+(self.h*p),name='right'))
        self.cplist.append(PrincipalConnectionPoint((self.x+self.w*p),self.y,side=False,name='top'))
        self.cplist.append(PrincipalConnectionPoint((self.x+self.w*p),self.y+self.h,side=False,name='bottom'))
        
        inc=0.1*self.h
        id=1
        for i in range(1,5):
            self.cplist[0].addPoint(ConnectionPoint(self.x,self.y+0.5*self.h+inc*i,True,id))
            self.cplist[1].addPoint(ConnectionPoint(self.x+self.w,self.y+0.5*self.h+inc*i,True,id))
            id=id+1
            self.cplist[0].addPoint(ConnectionPoint(self.x,self.y+0.5*self.h-inc*i,True,id))
            self.cplist[1].addPoint(ConnectionPoint(self.x+self.w,self.y+0.5*self.h-inc*i,True,id))
            id=id+1
 
        inc=10
        id=1
        for i in range(1,10): 
            if (i*inc)<(0.5*self.w):
                self.cplist[2].addPoint(ConnectionPoint((self.x+0.5*self.w-(i*inc)),self.y,False,id))
                self.cplist[3].addPoint(ConnectionPoint((self.x+0.5*self.w-(i*inc)),self.y+self.h,False,id))
                id=id+1
                self.cplist[2].addPoint(ConnectionPoint((self.x+0.5*self.w+(i*inc)),self.y,False,id))
                self.cplist[3].addPoint(ConnectionPoint((self.x+0.5*self.w+(i*inc)),self.y+self.h,False,id))
                id=id+1

    def refreshConnectionPoints(self):
        for cp in self.cplist:
            cp.setFree()
    
    def encloses(self,x,y):
        return (x>=self.x and x<=self.x+self.w)and (y>=self.y and y<=self.y+self.h) 

    def enclosesPosition(self,position):
        return self.encloses(position[0],position[1])

    def enclosesPoint(self,p):
        return self.encloses(p.getX(),p.getY())

 
class rectangle_point(rectangle):
    def __init__(self,p,w=DEFAULT_W,h=DEFAULT_H):
        rectangle.__init__(self,p.getX(),p.getY(),w,h)

class textlabel:
    def __init__(self,text,p,w):
        self.textXOffset=5
        self.x = p.getX() + self.textXOffset
        self.y=p.getY()
        self.w =w
        self.text=text
        
    def draw(self,dc):
        dc.BeginDrawing()
        allowedWidth=(self.w - (2*self.textXOffset))
        dc.DrawTextPoint(getDisplayText(self.text,allowedWidth,dc),(self.x,self.y))
        dc.EndDrawing()

class arrow:

    def __init__(self,v,size,p=0.5):
        self.size=size
        self.v=v
        self.proportionPoint=p
    
    def draw(self,dc):
        
        if self.v.isZero():
            return
            
        points=[]
        points.extend(self.getArrow(self.v,self.size))
        lines=[]
        lines.extend(points)
        dc.BeginDrawing()
        dc.DrawLineList(lines)
        dc.EndDrawing()
       
    def getArrow(self,v,size):
        """
        Given a line described by vector v, returns points defining an arrow head at
        the midpoint of the line.
        """
       
        len=v.getLength()
        mid=v.getPoint(self.proportionPoint)
        arrowA=mathutils.vector(mid,mid-( (v.getUnitVector()+v.getNormalUnitVector())*size ) )
        arrowB=mathutils.vector(mid,mid-( (v.getUnitVector()-v.getNormalUnitVector())*size ) )
        
        return [arrowA.asTuple(),arrowB.asTuple()] 

class iconItem:
    def __init__(self,icon,x,y):
        self.icon=icon
        self.x=x
        self.y=y

    def setXOffset(self,offset):
        self.x=self.x-offset

    def draw(self,dc):
        dc.DrawBitmap(self.icon,self.x,self.y)

class titleItem:
    def __init__(self,text,x,y):
        self.text=text
        self.x=x
        self.y=y

    def setXOffset(self,offset):
        self.x=self.x-offset

    def draw(self,dc):
        pts,weight=fonts.setTableTitleFont(dc)
        dc.DrawText(self.text,self.x,self.y)
        fonts.resetFont(dc,pts,weight)

class filledTitleItem:
    def __init__(self,text,x,y,w,h):
        self.text=text
        self.x=x
        self.y=y
        self.h=h
        self.w=w
        self.oldXOffset=0
        self.oldYOffset=0

    def setXOffset(self,offset):
        dxOffset=offset-self.oldXOffset
        self.oldXOffset=offset
        self.x=self.x-dxOffset
    
    def setYOffset(self,offset):
        dyOffset=offset-self.oldYOffset
        self.oldYOffset=offset
        self.y=self.y-dyOffset

    def draw(self,dc):
        print 'filledTitleItem:draw'
        pts,weight=fonts.setTableTitleFont(dc)
        dc.GradientFillLinear(wx.Rect(self.x,self.y,self.w,self.h),colours.titleBarGradient1,colours.titleBarGradient2,wx.SOUTH)
        x=self.x+0.5*(self.w-(dc.GetTextExtent(self.text)[0]))
        dc.DrawText(self.text,x,self.y)
        fonts.resetFont(dc,pts,weight)

class columnHeader:
    def __init__(self,text,x,y,w):
        self.text=text
        self.x=x
        self.y=y
        self.w=w
        self.oldXOffset=0
        self.oldYOffset=0

    def setXOffset(self,offset):
        dxOffset=offset-self.oldXOffset
        self.oldXOffset=offset
        self.x=self.x-dxOffset
    
    def setYOffset(self,offset):
        dyOffset=offset-self.oldYOffset
        self.oldYOffset=offset
        self.y=self.y-dyOffset

    def draw(self,dc):
        print 'columnHeader:draw'
        pts,weight=fonts.setSubTableTitleFont(dc)
        h=dc.GetTextExtent('0')[1]
        dc.GradientFillLinear(wx.Rect(self.x,self.y,self.w,h),colours.titleBarGradient1,colours.titleBarGradient2,wx.SOUTH)
        x=self.x+0.5*(self.w-(dc.GetTextExtent(self.text)[0]))
        dc.DrawText(self.text,x,self.y)
        fonts.resetFont(dc,pts,weight)


class darkDivider:
    def __init__(self,x1,y1,x2,y2):
        self.x1=x1
        self.x2=x2
        self.y1=y1
        self.y2=y2

    def setXOffset(self,offset):
        self.x1=self.x1-offset
        self.x2=self.x2-offset

    def draw(self,dc):
        setPenColour(dc,colours.dark_gridlines)
        dc.DrawLine(self.x1,self.y1,self.x2,self.y2)
        setPenColour(dc,colours.dictionaryDefaultPen)

class titleBar:
    def __init__(self,x,y,w,h):
        self.x=x
        self.w=w
        self.y=y
        self.h=h

    def setXOffset(self,offset):
        self.x=self.x-offset

    def draw(self,dc):
        dc.GradientFillLinear(wx.Rect(self.x+1,self.y,self.w-1,self.h),colours.titleBarGradient1,colours.titleBarGradient2,wx.SOUTH)

class headerGrid:
    def __init__(self,x,y,w,h):
        self.x=x
        self.w=w
        self.y=y
        self.h=h
        self.oldXOffset=0
        self.oldYOffset=0

    def setXOffset(self,offset):
        dxOffset=offset-self.oldXOffset
        self.oldXOffset=offset
        self.x=self.x-dxOffset
    
    def setYOffset(self,offset):
        dyOffset=offset-self.oldYOffset
        self.oldYOffset=offset
        self.y=self.y-dyOffset

    def draw(self,dc):
        pts,weight=fonts.setSubTableTitleFont(dc)
        h=dc.GetTextExtent('0')[1]
        setPenColour(dc,colours.dark_gridlines)
        dc.DrawLine(self.x,self.y+h,self.x+self.w,self.y+h)
        setPenColour(dc,colours.defaultPen)
        fonts.resetFont(dc,pts,weight)

class divider:
    def __init__(self,x1,y1,x2,y2):
        self.x1=x1
        self.x2=x2
        self.y1=y1
        self.y2=y2

    def setXOffset(self,offset):
        self.x1=self.x1-offset
        self.x2=self.x2-offset

    def draw(self,dc):
        setPenColour(dc,colours.dictionaryDivider)
        dc.DrawLine(self.x1,self.y1,self.x2,self.y2)
        setPenColour(dc,colours.dictionaryDefaultPen)

class textItem:
    def __init__(self,text,x,y,colour=colours.dictionaryText):
        self.text=text
        self.x=x
        self.y=y
        self.colour=colour

    def setXOffset(self,offset):
        self.x=self.x-offset

    def draw(self,dc,xOffset=0,yOffset=0):
        dc.SetTextForeground(self.colour)
        dc.DrawText(self.text,self.x-xOffset,self.y-yOffset)

class dictionaryLabel:
    def __init__(self,label,x,y):
        self.label=label
        self.x=x
        self.y=y

    def setXOffset(self,offset):
        self.x=self.x-offset

    def draw(self,dc):
        dc.SetTextForeground(colours.dictionaryLabelText)
        pts,weight=fonts.setDictionaryLabelFont(dc)
        dc.DrawText(self.label,self.x+10,self.y-10)
        dc.SetTextForeground('black')
        fonts.resetFont(dc,pts,weight)

class line:
    def __init__(self,v,directional=1,p=0.5,colour=wx.Colour(0,0,0),terminators=False):
        self.v=v
        self.directional=directional
        self.p=p
        #self.arrow=arrow(v,5,p)
        self.colour=colour
        self.terminators=terminators
        self.arrows=[]
        self.routePoints=[]
            
    def __eq__(self,other):
        
        return isinstance(other,line) and  self.v.getP1()==other.v.getP1() and self.v.getP2()==other.v.getP2()

    def setDirectional(self,directional):
        self.directional=directional

    def setColour(self,colour):
        self.colour=colour
    def setRoute(self,routePoints):
        self.routePoints=routePoints
        
    def draw(self,dc,xOffset=0,yOffset=0):
        t=self.v.asTuple()
        self.arrows=[]
        
        pointlist=[]
        pointlist.append(wx.Point(t[0]-xOffset,t[1]-yOffset))
        for p in self.routePoints:
            pointlist.append(wx.Point(p[0]-xOffset,p[1]-yOffset))
        pointlist.append(wx.Point(t[2]-xOffset,t[3]-yOffset))

        for i in range(len(pointlist)-1):
            p1=pointlist[i]
            p2=pointlist[i+1]
            #v=mathutils.vector(mathutils.point(p1.x-xOffset,p1.y-yOffset),mathutils.point(p2.x-xOffset,p2.y-yOffset))
            v=mathutils.vector(mathutils.point(p1.x,p1.y),mathutils.point(p2.x,p2.y))
            if v.getLength()>15: 
                a=arrow(v,5,self.p)
                self.arrows.append(a)
        
        pen=dc.GetPen()
        pen.SetColour(self.colour)
        dc.SetPen(pen)
#        dc.BeginDrawing()
        dc.DrawLines(pointlist)
#        dc.EndDrawing()

        if self.directional==1: 
            for a in self.arrows:
                a.draw(dc)
        elif self.directional==2:
            if self.arrows:
                self.arrows[0].draw(dc)

        if self.terminators:
            brush=dc.GetBrush()
            brush.SetColour(self.colour)
            dc.SetBrush(brush)
            dc.DrawCircle(t[0]-xOffset,t[1]-yOffset,3)
            dc.DrawCircle(t[2]-xOffset,t[3]-yOffset,3)
            brush=dc.GetBrush()
            brush.SetColour(wx.Colour('white'))
            dc.SetBrush(brush)

        pen.SetColour('BLACK')
        dc.SetPen(pen)

class connectingline(line):
    def __init__(self,v,directional=1,p=0.5,colour=wx.Colour(0,0,0),terminators=False, idList=[],layer=0):
        self.idList=idList
        self.layer=layer
        line.__init__(self,v,directional=directional,p=p,colour=colour,terminators=terminators)

        
class HScrollMeter:
    def __init__(self,start,length,size,pos):
        self.start=start
        self.length=length
        self.size=size
        self.pos=pos
       
    def draw(self,dc):
        dc.SetPen(wx.Pen(colours.defaultScrollMeterEdge))
        dc.SetBrush(wx.Brush('WHITE'))
        dc.DrawRectangle(self.pos[0],self.pos[1]+0.1*self.size[1],self.size[0],self.size[1])
        dc.SetBrush(wx.Brush(colours.defaultScrollMeterFill))
        dc.DrawRectangle(self.start,self.pos[1]+0.1*self.size[1],self.length,self.size[1])
        dc.SetBrush(wx.Brush('WHITE'))
        dc.SetPen(wx.Pen('BLACK'))

class VScrollMeter:
    def __init__(self,start,length,size,pos):
        self.start=start
        self.length=length
        self.size=size
        self.pos=pos
       
    def draw(self,dc):
        dc.SetPen(wx.Pen(colours.defaultScrollMeterEdge))
        dc.SetBrush(wx.Brush('WHITE'))
        dc.DrawRectangle(self.pos[0],self.pos[1],self.size[0],self.size[1])
        dc.SetBrush(wx.Brush(colours.defaultScrollMeterFill))
        dc.DrawRectangle(self.pos[0],self.start,self.size[0],self.length)
        dc.SetBrush(wx.Brush('WHITE'))
        dc.SetPen(wx.Pen('BLACK'))

