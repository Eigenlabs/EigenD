
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
from pigui import colours

class LowerLeftSidePanel(wx.Window):
    def __init__(self,parent,size,style=wx.BORDER_NONE):
        wx.Window.__init__(self,parent,-1,size=size,style=style)  
        self.Bind(wx.EVT_MOUSE_EVENTS,self.doNothing)
        self.SetBackgroundColour(colours.horPanelGradient2)
        self.Bind(wx.EVT_PAINT, self.onPaint)

    def doNothing(self,evt):
        # do nothing
        pass

    def onPaint(self,evt):
        self.doPaint()
        evt.Skip()

    def doPaint(self):
        dc=wx.PaintDC(self)
        size=self.GetClientSize()
        dc.GradientFillLinear(wx.Rect(0,0,size[0],size[1]),colours.horPanelGradient2,colours.horPanelGradient1,wx.SOUTH)


class LowerRightSidePanel(wx.Window):
    def __init__(self,parent,size,style=wx.BORDER_NONE):
        wx.Window.__init__(self,parent,-1,size=size,style=style)  
        self.Bind(wx.EVT_MOUSE_EVENTS,self.doNothing)
        self.SetBackgroundColour(colours.horPanelGradient2)
        self.Bind(wx.EVT_PAINT, self.onPaint)

    def doNothing(self,evt):
        # do nothing
        pass

    def onPaint(self,evt):
        self.doPaint()
        evt.Skip()

    def doPaint(self):
        dc=wx.PaintDC(self)
        size=self.GetClientSize()
        dc.GradientFillLinear(wx.Rect(0,0,size[0],size[1]),colours.horPanelGradient2,colours.horPanelGradient1,wx.SOUTH)

class LeftSidePanel(wx.Window):
    def __init__(self,parent,size,style=wx.BORDER_NONE):
        wx.Window.__init__(self,parent,-1,size=size,style=style)  
        self.Bind(wx.EVT_MOUSE_EVENTS,self.doNothing)
        self.SetBackgroundColour(colours.horPanelGradient2)
        self.Bind(wx.EVT_PAINT, self.onPaint)

    def doNothing(self,evt):
        # do nothing
        pass

    def onPaint(self,evt):
        self.doPaint()
        evt.Skip()

    def doPaint(self):
        dc=wx.PaintDC(self)
        size=self.GetClientSize()
        dc.GradientFillLinear(wx.Rect(0,0,size[0],size[1]),colours.horPanelGradient2,colours.horPanelGradient1,wx.SOUTH)


class RightSidePanel(wx.Window):
    def __init__(self,parent,size,style=wx.BORDER_NONE):
        wx.Window.__init__(self,parent,-1,size=size,style=style)  
        self.Bind(wx.EVT_MOUSE_EVENTS,self.doNothing)
        self.SetBackgroundColour(colours.horPanelGradient2)
        self.Bind(wx.EVT_PAINT, self.onPaint)

    def doNothing(self,evt):
        # do nothing
        pass

    def onPaint(self,evt):
        self.doPaint()
        evt.Skip()

    def doPaint(self):
        dc=wx.PaintDC(self)
        size=self.GetClientSize()
        dc.GradientFillLinear(wx.Rect(0,0,size[0],size[1]),colours.horPanelGradient2,colours.horPanelGradient1,wx.SOUTH)


class BottomPanel(wx.Window):
    def __init__(self,parent,size,style=wx.BORDER_NONE):
        wx.Window.__init__(self,parent,-1,size=size,style=style)  
        self.Bind(wx.EVT_MOUSE_EVENTS,self.doNothing)
        self.SetBackgroundColour(colours.frame_border)
        self.Bind(wx.EVT_PAINT, self.onPaint)

    def doNothing(self,evt):
        # do nothing
        pass

    def onPaint(self,evt):
        self.doPaint()
        evt.Skip()

    def doPaint(self):
        size=self.GetClientSize()
        position=self.GetPosition()
        dc=wx.PaintDC(self)
        dc.Clear()
        dc.GradientFillLinear(wx.Rect(0,0,size[0],size[1]),colours.horPanelGradient1,colours.horPanelGradient2,wx.SOUTH)

    def updateStatus(self,text):
        pass


class TopPanel(wx.Window):
    def __init__(self,parent,size,agent,style=wx.BORDER_NONE):
        wx.Window.__init__(self,parent,-1,size=size,style=style)  
        self.Bind(wx.EVT_MOUSE_EVENTS,self.doNothing)
        self.SetBackgroundColour(colours.frame_border)
        self.title=''
        self.model=agent[8].model
        self.model.addTitleListener(self)
        self.Bind(wx.EVT_PAINT, self.onPaint)

    def setLabel(self,label):
        print 'setlabel',label
        self.title=label
        self.doPaint(wx.ClientDC(self))
        self.Refresh()
    
    def doNothing(self,evt):
        # do nothing
        pass

    def update(self):
        print 'TopPanel update'
        title=''
        s=self.getTitle()
        if s:
            title='Browsing ' +s
        self.setLabel(title)

    def onPaint(self,evt):
        dc=wx.PaintDC(self)
        self.doPaint(dc)
        evt.Skip()

    def doPaint(self,dc):
        font=wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        font.SetPointSize(11)
	dc.SetFont(font)

        size=self.GetClientSize()
        dc.GradientFillLinear(wx.Rect(0,0,size[0],size[1]),colours.horPanelGradient1,colours.horPanelGradient2,wx.SOUTH)
        dc.DrawText(self.title,11,5)

    def getTitle(self):
        targetName=self.model.targetName
        title=''
        if targetName:
#            if (not self.model.keyval) and self.model.numFiles==0 and self.model.numCollections==0:
#                title=targetName+ ' : None found'
#            else:
#                title=targetName
            title=targetName
        
        return title

    def __doTitlePanelDraw(self):
        s=self.getTitle()
        if s:
            self.setLabel('Browsing '+s)
        else:
            self.setLabel('')


class HorDivPanel(wx.Window):
    def __init__(self,parent,size,style=wx.BORDER_NONE):
        wx.Window.__init__(self,parent,-1,size=size,style=style)  
        self.Bind(wx.EVT_MOUSE_EVENTS,self.doNothing)
        self.SetBackgroundColour(colours.frame_border)
        self.Bind(wx.EVT_PAINT, self.onPaint)

    def doNothing(self,evt):
        # do nothing
        pass

    def onPaint(self,evt):
        dc=wx.PaintDC(self)
        self.doPaint(dc)
        evt.Skip()

    def doPaint(self,dc):
        size=self.GetClientSize()
        dc.GradientFillLinear(wx.Rect(0,0,size[0],size[1]),colours.horPanelGradient1,colours.horPanelGradient2,wx.SOUTH)


class VertDivPanel(wx.Window):
    def __init__(self,parent,size,style=wx.BORDER_NONE):
        wx.Window.__init__(self,parent,-1,size=size,style=style)  
        self.Bind(wx.EVT_MOUSE_EVENTS,self.doNothing)
        self.SetBackgroundColour(colours.frame_border)
        self.Bind(wx.EVT_PAINT, self.onPaint)

    def doNothing(self,evt):
        # do nothing
        pass

    def onPaint(self,evt):
        dc=wx.PaintDC(self)
        self.doPaint(dc)
        evt.Skip()

    def doPaint(self,dc):
        size=self.GetClientSize()
        dc.GradientFillLinear(wx.Rect(0,0,size[0],size[1]),colours.horPanelGradient2,colours.horPanelGradient1,wx.SOUTH)

