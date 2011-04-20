
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

import wx
from pigui import colours,utils,drawutils

class InfoPanel(wx.Window):
    def __init__(self,parent,size,agent,style=wx.BORDER_NONE|wx.VSCROLL):
        wx.Window.__init__(self,parent,-1,size=size,style=style)  
        self.model=agent[8].model
        self.model.addInfoListener(self)

	self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)
        self.SetBackgroundColour(colours.borderGradient1)
        self.__agent=agent
        self.Bind(wx.EVT_LEFT_DOWN,self.onLeftDown)
        self.Bind(wx.EVT_LEFT_UP,self.onLeftUp)
        self.Bind(wx.EVT_MOTION,self.onMotion)
        #self.SetBackgroundColour(colours.panel_background)
        self.Bind(wx.EVT_PAINT, self.onPaint)
        self.Bind(wx.EVT_SCROLLWIN, self.onScroll)
        self.Bind(wx.EVT_MOUSEWHEEL,self.onMouseWheel)

        self.yScroll=1
        self.yScrollCentre=1
        self.oldv=1
        self.offset=0
        self.__selection=0
        self.__selection_changed=False
        self.__numItemsDisplayed=0

        self.__truncatedValues=[]
        self.showScrollbar=False
        self.val=0
        self.absPos=0

        self.SetToolTip(wx.ToolTip(''))
        self.SetScrollbar(wx.VERTICAL,0,0,0)

    def onScroll(self,evt):
        offset=evt.GetPosition()
        self.scrollbarScroll(offset)

    def onMouseWheel(self,evt):
        delta=evt.GetWheelDelta()
        rotation=evt.GetWheelRotation()
#        print 'onMouseWheel:delta=',delta,'rotation=',rotation
        deltaOffset=rotation*delta*evt.GetLinesPerAction()
        self.wheelScroll(deltaOffset)

    def onLeftDown(self,evt):
        pass
        
    def onLeftUp(self,evt):
        pass

    def targetChanged(self):
        print 'InfoPanel:TargetChanged'
        self.offset=0
        self.yScroll=1
        self.oldv=1
        self.offset=0
        self.__selection=0
        self.__selection_changed=False
        self.__truncatedValues=[]
        self.__agent.reset2(1,1)
   
    def onMotion(self,evt):
        mPos=evt.GetPositionTuple()
        
        for v in self.__truncatedValues:
            if mPos[0]>v[0] and mPos[0]<v[1] and mPos[1]>v[2] and mPos[1]<v[3]:
                self.GetToolTip().SetTip(v[4])
                return 
    
        self.GetToolTip().SetTip('')

    def resetVScroller(self,v):
        self.__agent.reset2(1,v)
 
    def mouseScroll(self,v):
        print 'mousescroll',v
        self.resetVScroller(v)
        self.scroll_both(1,v,mouse=True)

    def onPaint(self,evt):
        #dc=wx.PaintDC(self)
	dc=self.__getClientDC()
        self.doPaint(dc)
        evt.Skip()

    def update(self):
        print 'InfoPanel:update'
        #dc=wx.ClientDC(self)
	dc=self.__getClientDC()
        self.doPaint(dc)

    def __getClientDC(self):
	if self.IsDoubleBuffered():
           dc=wx.ClientDC(self)
	else:
	   dc=wx.BufferedDC(wx.ClientDC(self))
        dc.Clear()

        font=wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        font.SetPointSize(11)
	dc.SetFont(font)

        brush=dc.GetBrush()
        brush.SetColour(colours.borderGradient1)
        dc.SetBackground(brush)
	return dc

    def doPaint(self,dc):
        size=self.GetClientSize()
	drawutils.setPenColour(dc,colours.borderGradient1)
	drawutils.setBrushColour(dc,colours.borderGradient1)
        dc.DrawRectangle(0,0,size[0],size[1])

        self.__truncatedValues=[]
        if self.model.keyval:
            self.__keyval_drawing(dc)

        self.__borderDrawing(dc)

    def __borderDrawing(self,dc):
        size=self.GetClientSize()
        pen=wx.Pen(colours.border_shadow_dark)
        pen.SetWidth(2)
        pen.SetCap(wx.CAP_BUTT)
        dc.SetPen(pen)
        dc.DrawLine(1,0,1,size[1])
        dc.DrawLine(1,size[1]-1,size[0],size[1]-1)
        pen=wx.Pen(colours.defaultPen)
        pen.SetWidth(1)
        dc.SetPen(pen)

    def wheelScroll(self,deltaOffset):
        offset=self.offset-deltaOffset
        numLines=self.lineCount
        if offset>=0 and offset<=((numLines+1)-self.__numItemsDisplayed):
            self.offset=offset
            self.__scrollDraw()

    def __keyval_drawing(self,dc):
        keyval=self.model.keyval
        self.lineCount=0
        if keyval:
            size=self.GetClientSize()
            x=0
            y=5
            yinc=1.2*dc.GetTextExtent('0')[1]
            maxk=0
            for k,v in keyval:
                test=dc.GetTextExtent(str(k))[0]+10
                if test>maxk:
                    maxk=test
            maxk=min(maxk,size[0]*0.5)    
                
            for k,v in keyval[self.offset:]:
                dc.GradientFillLinear(wx.Rect(1,y-1,size[0],yinc),colours.borderGradient1,colours.borderGradient2,wx.SOUTH)
                dc.DrawText(str(k), maxk-(dc.GetTextExtent(str(k))[0]+5),y)

                displayText=[]
                maxWidth=0.9* (size[0]-(maxk+5))
                widthTolerance=0.2*maxWidth 
                #utils.chopString(str(v),dc,maxWidth,displayText,widthTolerance,True)
                utils.chopString2(str(v),dc,maxWidth,displayText,0,True)
                extraLine=False
                for choppedLine in displayText:
                    if extraLine:
                        dc.GradientFillLinear(wx.Rect(1,y-1,size[0],yinc),colours.borderGradient1,colours.borderGradient2,wx.SOUTH)
                    extraLine=True
                    dc.DrawText(choppedLine, maxk+5,y)
                    self.lineCount=self.lineCount+1

                    #if dc.GetTextExtent(str(v))[0]>size[0]-(maxk+5):
                    #    self.__truncatedValues.append((maxk,size[0],y,y+yinc,str(v)))
                    drawutils.setPenColour(dc,colours.browser_gridlines)
                    dc.DrawLine(maxk,y,maxk,y+yinc)
                    drawutils.setPenColour(dc,'black')
                    #count=count+1
                    y=y+yinc

            if self.lineCount*yinc>size[1]:
                self.showScrollbar=True
            self.__numItemsDisplayed=int(size[1]/yinc)
            self.SetScrollbar(wx.VERTICAL,self.offset,self.__numItemsDisplayed,self.lineCount+1)

    def scroll_both(self,h,v,mouse=False):
        if self.showScrollbar:
#            print 'InfoPanel: scroll_both', h,v,mouse
            if h is not None: self.xScroll=h
            if v is not None: self.yScroll=v
            keyval =self.model.keyval
            if keyval:
                if abs(self.oldv-v)>0.0001:
                    numLines=self.lineCount
                    if numLines>5:
                        p=5
                    else:
                        p=1
                    ys=int((-0.5*(v-1))*(numLines-p))
                    selection=int((-0.5*(v-1))*(numLines-1))
                    if selection!=self.__selection:
                        self.__selection_changed=True
                    self.__selection=selection
                    if self.__scrolling_up(v):
                        if self.__selection>=(self.offset+self.__numItemsDisplayed):
                            self.offset=ys
                    elif self.__scrolling_down(v):
                        if (self.__selection-self.offset)<=0: 
                            self.offset=ys
                    self.oldv=v
                    self.__scrollDraw()

    def __scrollDraw(self):
        #dc=wx.ClientDC(self)
	dc=self.__getClientDC()
        self.__doScrollDraw(dc)
        self.__borderDrawing(dc)

    def scrollbarScroll(self,offset):
        self.offset=offset
        self.__scrollDraw()

    def __doScrollDraw(self,dc):
#        print 'InfoPanel:__doScrollDraw'
        self.doPaint(dc)
    
    def __scrolling_down(self,v):
        return v>self.oldv

    def __scrolling_up(self,v):
        return v<self.oldv


