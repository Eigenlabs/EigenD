
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
from pigui import colours,fonts,utils,drawutils

class BelcantoPanel(wx.Window):
    def __init__(self,parent,size,agent,style=wx.BORDER_NONE):
        wx.Window.__init__(self,parent,-1,size=size,style=style)  
        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)
        self.model=agent.historyModel
        self.model.addHistoryListener(self)
        self.SetBackgroundColour(colours.borderGradient1)
        self.__agent=agent
        self.Bind(wx.EVT_PAINT, self.onPaint)
        self.font=wx.Font(13,wx.FONTFAMILY_MODERN,wx.FONTSTYLE_NORMAL,weight=wx.FONTWEIGHT_LIGHT)

    def __getClientDC(self):
        if self.IsDoubleBuffered():
           dc=wx.ClientDC(self)
        else:
           dc=wx.BufferedDC(wx.ClientDC(self))
        dc.Clear()
        brush=dc.GetBrush()
        brush.SetColour(colours.borderGradient1)
        dc.SetBackground(brush)
        dc.SetFont(self.font)
        return dc

    def onPaint(self,evt):
        dc=self.__getClientDC()
        self.doPaint(dc)
        evt.Skip()

    def historyUpdate(self):
        print 'BelcantoPanel:update'
        if self.model.words:
            dc=self.__getClientDC()
            self.doPaint(dc)

    def updateStatus(self,str):
        print 'BelcantoPanel:updateStatus',str
#        self.agent.updateStatus(str)
#        self.status=str
#        self.update()

    def doPaint(self,dc):
        self.__backgroundDrawing(dc)
        y=2
        x=2
        
        print 'belcanto_panel:doPaint',self.model.words
        pts,weight=fonts.setFont(dc,ptfactor=0.75)
        displayText=[]
        maxWidth=0.95*self.GetClientSize()[0]
        widthTolerance=0.1*maxWidth
        #utils.chopString(self.model.get_text(),dc,maxWidth,displayText,widthTolerance,True)
        utils.chopString2(self.model.get_text(),dc,maxWidth,displayText,0,True)
        
        yInc=1.1*dc.GetTextExtent('A')[1]
        for choppedLine in displayText:
            dc.DrawText(choppedLine,x,y)
            print 'BelcantoPanel',choppedLine
            y=y+yInc

        self.__borderDrawing(dc)
        fonts.resetFont(dc,pts,weight)
    
    def __backgroundDrawing(self,dc):
        size=self.GetClientSize()
        drawutils.setPenColour(dc,colours.borderGradient1)
        drawutils.setBrushColour(dc,colours.borderGradient1)
        dc.DrawRectangle(0,0,size[0],size[1])
 
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


