
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

import time
import wx
import piw
from pi import atom,action,domain
from pigui import colours,drawutils

class ViewModel:
    def __init__(self):
        self.listeners=[]

    def addListener(self,listener):
        if not listener in self.listeners:
            self.listeners.append(listener)

    def removeListener(self,listener):
        if listener in self.listeners:
            self.listeners.remove(listener)

    def update(self):
        for listener in self.listeners:
            listener.update()

    def redraw(self):
        for listener in self.listeners:
            listener.redrawUpdate()

class MouselessPanel(wx.Window):
    def __init__(self,parent,size):
        wx.Window.__init__(self,parent,-1,size=size,style=wx.BORDER_NONE)  
       # wx.Window.__init__(self,parent,-1,size=size,style=wx.BORDER_SUNKEN)  
        self.Bind(wx.EVT_MOUSE_EVENTS,self.doNothing)
        self.SetCursor(wx.StockCursor(wx.CURSOR_NO_ENTRY))
        self.paint_me=True

    def display(self,paint):
        self.paint_me=paint

    def doNothing(self,evt):
        # do nothing
        pass

class ViewDrawing:
    def __init__(self,factory,parentPanel):
        self.factory=factory
        self.parentPanel=parentPanel
        self.active=True

        self.xScroll=-1
        self.yScroll=0
        self.xScrollCentre=-1
        self.yScrollCentre=-1

        self.minX=0
        self.maxX=0
        self.minY=0
        self.maxY=0

        self.centre=False
        self.drawRequired=False
        self.redrawRequired=False

    def tap(self,n):
        pass

    def calcCentre(self):
        self.doCentre()

    def clearCache(self):
        pass

    def isDrawRequired(self):
        return v.drawRequired

    def setDrawRequired(self,drawRequired):
        self.drawRequired=drawRequired

    def doDrawingIfRequired(self):
        if self.drawRequired:
            self.setDrawRequired(False)
            self.setRedrawRequired(False)
            self.draw1()

    def doRedrawingIfRequired(self):
        if self.redrawRequired:
            self.setRedrawRequired(False)
            self.redraw1()

    def isDrawRequired(self):
        return self.drawRequired

    def setDrawRequired(self,drawRequired):
        self.drawRequired=drawRequired

    def isRedrawRequired(self):
        return self.redrawRequired 

    def setRedrawRequired(self,redrawRequired):
        self.redrawRequired=redrawRequired

    def resetVScroller(self,v):
        self.factory.reset1(self.xScroll,v)
     
    def resetHScroller(self,h):
        self.factory.reset1(h,self.yScroll)
 
    def doCentre(self):
        self.xScroll=self.xScrollCentre
        self.yScroll=self.yScrollCentre
        print 'doCentre:setting xScroll to', self.xScroll,'yScroll to', self.yScroll
        self.centre=False
        self.factory.reset1(self.xScroll,self.yScroll)

    def initBuffer(self):
        self.preDraw()

    def setArgs(self,args):
        pass

    def doDrawing(self,dc):
        pass

    def update(self):
        self.preDraw()

    def redrawUpdate(self):
        self.setRedrawRequired(True)

    def preDraw(self):
        pass

    def getXScrollPos(self,minLimit=True):
        d=self.xScroll
        xScrollPos=(d+1)*0.5*(self.__xMax()-self.__xMin())+self.__xMin()
        raw=xScrollPos

        if minLimit:
            if xScrollPos<0:
                xScrollPos=0
               
        xScrollPos=int(xScrollPos)
#        print 'getXScrollPos: xScrollPos',xScrollPos,'maxX',self.__xMax(),'minX',self.__xMin()
        return xScrollPos

    def getYScrollPos(self,minLimit=True):
        d=self.yScroll
#        print 'getYScrollPos: __yMin(),__yMax(),d',self.__yMin(),self.__yMax(),d
        yScrollPos=(-d+1)*0.5*(self.__yMax()-self.__yMin())+self.__yMin()
        raw1=yScrollPos
        if minLimit:
            if yScrollPos<0:
                yScrollPos=0
        raw2=yScrollPos
        #if yScrollPos>(self.h-self.parentPanel.GetSize()[1]):
        #    yScrollPos=self.h-self.parentPanel.GetSize()[1]
        yScrollPos=int(yScrollPos)
#        print 'getYScrollPos: yScrollPos',yScrollPos,raw1,raw2
        #print 'getYScrollPos: yScrollPos',yScrollPos,'maxY',self.__yMax(),'minY',self.__yMin()
        return yScrollPos

    def __xMin(self):
        return self.minX

    def __xMax(self):
        return self.maxX

    def __yMax(self):
        return self.maxY
        
        #yMax= (self.maxY+self.yMaxScrollMargin)-self.parentPanel.GetSize()[1]
        #return yMax

    def __yMin(self):
        return self.minY
#        ymin=self.minY
#        if ymin>50:
#            ymin=ymin-50
#        return ymin

    def getMinX(self,dc):
        return dc.MinX()

    def getMinY(self,dc):
        return dc.MinY()

    def getMaxX(self,dc):
        return dc.MaxX()

    def getMaxY(self,dc):
        return dc.MaxY()
 
    def getHelp(self):
        return []

    def GetSize(self):
        #size = self.parentPanel.GetSize()
        size = self.parentPanel.GetClientSizeTuple()
        return size

    def initDC(self,dc):
        dc.SetFont(self.factory.font)
        brush=dc.GetBackground()
        brush.SetColour('white')
        dc.SetBackground(brush)
        dc.Clear()

    def getSizingFactor(self):
        dc=self.parentPanel.getClientDC()
        dc.SetFont(self.factory.font)
        return dc.GetTextExtent('0')[1]

    def getLineHeight(self,dc):
        dc.SetFont(self.factory.font)
        return int(1.1*dc.GetTextExtent("1")[1])

    def incrementYpos(self,dc):
        self.ypos=self.ypos+self.getLineHeight(dc)

    def close(self):
        self.active=False

    def activate(self,active):
        self.active=active

    def getScrollValues(self):
        return (self.xScroll,self.yScroll) 

    def draw1(self):
        print 'viewdrawing draw1'
        dc=wx.ClientDC(self.parentPanel)
        self.initDC(dc)
        self.doDrawing(dc)
        self.afterDraw(dc)
#        if self.centre:
        # XXX have to reset scroll position after a draw at present
        self.doCentre()
 
    def afterDraw(self,dc):
        self.minX=self.getMinX(dc)
        self.maxX=self.getMaxX(dc)
        self.minY=self.getMinY(dc)
        self.maxY=self.getMaxY(dc)

    def redraw1(self):
        print 'viewdrawing redraw1'
        dc=wx.ClientDC(self.parentPanel)
        self.initDC(dc)
        self.doRedraw(dc)

    def doRedraw(self,dc):
        pass

    def paint(self):
        print 'viewdrawing paint'
        dc=wx.PaintDC(self.parentPanel)
        self.initDC(dc)
        self.doScrollDrawing(dc)
#        self.doDrawing(dc)
#        self.afterDraw(dc)

    def getViewPortSize(self):
        return self.parentPanel.GetSize()

    def __getYOrigin(self):
        return 0

    def __getXOrigin(self):
        return 0

    def scroll_centre(self):
        print 'view_drawing:scroll_centre'
        self.centre=True
        self.__scroll_draw()

    def scroll_both(self,h,v):
        #print 'scroll_both:h=',h,'v=',v
        if h is not None: self.xScroll=h
        if v is not None: self.yScroll=v
        self.__scroll_draw()
        self.parentPanel.Update()

    def aux_scroll_both(self,h,v):
        pass

    def __scroll_draw(self):
        dc=wx.ClientDC(self.parentPanel)
        self.initDC(dc)
        self.doScrollDrawing(dc)

    def doScrollDrawing(self,dc):
        pass
    

class BrowserPanel(MouselessPanel):
    def __init__(self,parent,size):
        MouselessPanel.__init__(self,parent,size)
        self.Bind(wx.EVT_PAINT,self.OnPaint)
        self.Bind(wx.EVT_IDLE,self.onIdle)
#        self.timer=wx.Timer(self,1001)
#        self.Bind(wx.EVT_TIMER,self.onTimer,id=1001)
#        self.timer.Start(1000)
        self.view=None

#    def onTimer(self,evt):
#        if self.view:
#            self.view.doDrawingIfRequired()
#            self.view.doRedrawingIfRequired()

    def onIdle(self,evt):
        if self.view:
            self.view.doDrawingIfRequired()
            self.view.doRedrawingIfRequired()

    def forceDraw(self):
        self.view.draw1()

    def setViewDrawing(self,view):
        if self.view:
            self.view.activate(False)
        self.view=view
        self.view.activate(True)
 
    def OnPaint(self,evt):
        self.view.paint()

    def getClientDC(self):
        return wx.ClientDC(self)

    def getPaintDC(self):
        return wx.PaintDC(self)

    def getHelp(self):
        if self.view:
            return self.view.getHelp()
        else:
            return []

    def getTitle(self):
        if self.view:
            return self.view.getTitle()
        else:
            return ''

    def tap(self,n):
        if self.view:
            self.view.tap(n)

    def scroll_both(self,h,v):
        if self.view:
            self.view.scroll_both(h,v)

    def aux_scroll_both(self,h,v):
        if self.view:
            self.view.aux_scroll_both(h,v)
    
    def scroll_centre(self):
        if self.view:
            self.view.scroll_centre()


class ViewPanel(MouselessPanel):
    def __init__(self,parent,size):
        MouselessPanel.__init__(self,parent,size)
        self.agent=None
        self.Bind(wx.EVT_PAINT,self.OnPaint)
        self.preferredSize=(700,700)
 
    def OnPaint(self,evt):
        if self.paint_me:
            dc=self.getPaintDC()
            self.initDC(dc)
            self.draw(dc)

    def getClientDC(self):
        return wx.ClientDC(self)

    def getPaintDC(self):
        return wx.PaintDC(self)

    def update(self):
        if self.paint_me:
            dc=self.getClientDC()
            self.initDC(dc)
            self.draw(dc)

    def draw(self,dc):
        pass

    def getHelp(self):
        return []

    def initDC(self,dc):
        if self.agent:
            dc.SetFont(self.agent.font)
        self.setupBackground(dc)
        dc.Clear()
    
    def setupBackground(self,dc):
        brush=dc.GetBackground()
        brush.SetColour(colours.defaultViewBackground)
        dc.SetBackground(brush)
 
    def getSizingFactor(self):
        dc=wx.ClientDC(self)
        dc.SetFont(self.agent.font)
        return dc.GetTextExtent('0')[1]

    def getLineHeight(self,dc):
        dc.SetFont(self.agent.font)
        return int(1.1*dc.GetTextExtent("1")[1])

    def incrementYpos(self,dc):
        self.ypos=self.ypos+self.getLineHeight(dc)

    def close(self):
        pass

class TitlePanel(ViewPanel):
    def __init__(self,parent,size):
        ViewPanel.__init__(self,parent,size)
        self.title=''
    def setTitle(self,title):
        self.title=title
    def draw(self,dc):
        if self.title.startswith('Image'):
            drawutils.drawWindowHeader(self,'',dc,blank=True) 
        else:
            drawutils.drawWindowHeader(self,self.title,dc)
            self.Refresh()

class SpacerPanel(wx.Window):
    def __init__(self,parent,size,style=wx.BORDER_SIMPLE):
        wx.Window.__init__(self,parent,-1,size=size,style=style)  
        self.Bind(wx.EVT_MOUSE_EVENTS,self.doNothing)
        self.SetCursor(wx.StockCursor(wx.CURSOR_NO_ENTRY))

    def doNothing(self,evt):
        # do nothing
        pass






