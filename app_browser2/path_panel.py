
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
from pi import resource
from pigui import colours,fonts,drawutils

class PathPanel(wx.Window):
    def __init__(self,parent,size,agent,style=wx.BORDER_NONE):
        wx.Window.__init__(self,parent,-1,size=size,style=style)  
	self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)
        self.SetBackgroundColour(colours.borderGradient1)
        self.Bind(wx.EVT_LEFT_DOWN,self.onLeftDown)
        self.Bind(wx.EVT_LEFT_UP,self.onLeftUp)
        self.Bind(wx.EVT_MOTION,self.onMotion)
        self.__agent=agent
        self.model=self.__agent[8].model
        self.model.addPathListener(self)
        self.pathButtons=[]
        self.Bind(wx.EVT_PAINT, self.onPaint)
        self.path=[]
        self.aux_oldv=1
        self.aux_oldh=-1
#        self.font=wx.Font(13,wx.FONTFAMILY_MODERN,wx.FONTSTYLE_NORMAL,weight=wx.FONTWEIGHT_LIGHT)
        self.font=wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        self.font.SetPointSize(11)


        imageName=resource.find_release_resource('app_browser2','green_button.png')
        if imageName:
            image=wx.Image(imageName,wx.BITMAP_TYPE_PNG)
            self.green=image.ConvertToBitmap()
        imageName=resource.find_release_resource('app_browser2','red_button.png')
        if imageName:
            image=wx.Image(imageName,wx.BITMAP_TYPE_PNG)
            self.red=image.ConvertToBitmap()
        imageName=resource.find_release_resource('app_browser2','yellow_button.png')
        if imageName:
            image=wx.Image(imageName,wx.BITMAP_TYPE_PNG)
            self.yellow=image.ConvertToBitmap()

        self.scrollLength=1
        self.scrollPos=1

    def update(self):
        print 'PathPanel update'
        self.setPath(self.model.getPath())

    def onLeftDown(self,evt):
        pass

    def onLeftUp(self,evt):
        mPos=evt.GetPositionTuple()
        for pb in self.pathButtons:
            x0,x1,c=pb.getLimits()
            if mPos[0]<x1 and mPos[0]>=x0:
                self.scrollPos=c
                self.__agent.reset3(self.__spToScrollerVal(c),1)
                self.__agent.onTap3() 
    
    def onMotion(self,evt):
        mPos=evt.GetPositionTuple()
        for pb in self.pathButtons:
            x0,x1,c=pb.getLimits()
            if mPos[0]<x1 and mPos[0]>=x0:
                if self.scrollPos!=c:
                    self.scrollPos=c
                    self.__agent.reset3(self.__spToScrollerVal(c),1)
                    self.__scrolldraw()
 
    def onPaint(self,evt):
        dc=wx.AutoBufferedPaintDC(self)
        self.doPaint(dc)
        evt.Skip()

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

    def setPath(self,path):
        print 'PathPanel',path
        if path and path[0]=='No Path':
            print 'No target' 
            self.path=[]
        else:
            mainPath=['Top level']
            mainPath.extend(path)
            self.path=mainPath
            self.scrollLength=len(self.path)
            self.scrollPos=len(self.path)
            self.__agent.reset3(1,1)
#        dc=wx.ClientDC(self)
	dc=self.__getClientDC()
#        dc.SetFont(self.font)
#        dc.Clear()
        self.doPaint(dc)
        self.Refresh()
            
    def tap(self,tap):
        self.__doTap()
    
    def __doTap(self):
        self.__tapDraw()

    def getPathPos(self):
        return self.scrollPos-1

    def __borderDrawing(self,dc):
        size=self.GetClientSize()
        pen=wx.Pen(colours.border_shadow_dark)
        pen.SetWidth(2)
        pen.SetCap(wx.CAP_BUTT)
        dc.SetPen(pen)
        dc.DrawLine(1,0,1,size[1])
        dc.DrawLine(1,size[1]-1,size[0],size[1]-1)

    def __backgroundDrawing(self,dc):
        size=self.GetClientSize()
	drawutils.setPenColour(dc,colours.borderGradient1)
	drawutils.setBrushColour(dc,colours.borderGradient1)
        dc.DrawRectangle(0,0,size[0],size[1])
    
    def doPaint(self,dc):
	self.__backgroundDrawing(dc)

        if self.path:
            self.pathButtons=[]

            count=1
            px=5
            for p in self.path[:-1]:
                pb=PathButton(self,px,0,p,STATUS_ORDINARY,count)
                self.pathButtons.append(pb)
                px=px+pb.getWidth(dc)
                count=count+1
            if self.path:
                p=self.path[-1]
                pb=PathButton(self,px,0,p,STATUS_SELECTED,count)
                self.pathButtons.append(pb)
                px=px+pb.getWidth(dc)

            xoffset=0
            size=self.GetClientSize()
            test=size[0]
            if px>test:
                xoffset=test-px
            self.__drawPathButtons(dc,xoffset)
        self.__borderDrawing(dc)

    def __drawPathButtons(self,dc,xoffset=0):
         count=1
         for pb in self.pathButtons:
            if count==len(self.pathButtons):
                pb.drawlink=False
            pb.draw(dc,xoffset)
            count=count+1
   
    def scroll_both(self,h,v):
        m=(self.scrollLength-1)*0.5
        c=(self.scrollLength*0.5)+1
        sp=int((h*m)+c)
        print 'pathPanel,scroll_both',sp,'h=',h,'scrollLength',self.scrollLength
        if sp!=self.scrollPos:
            self.scrollPos=sp
            self.__scrolldraw()

    def __spToScrollerVal(self,sp):
        m=(self.scrollLength-1)*0.5
        c=(self.scrollLength*0.5)+1
        if m ==0:
            return -1
        return (sp-c)/m           

    def __scrolldraw(self):
        dc=self.__getClientDC()
	self.__backgroundDrawing(dc)
        self.__borderDrawing(dc)

        for pb in self.pathButtons:
            if pb.index==self.scrollPos:
                if pb.status==STATUS_ORDINARY:
                    pb.status=STATUS_SCROLL
                elif pb.status==STATUS_SCROLL:
                    pb.status=STATUS_ORDINARY
                elif pb.status==STATUS_SELECTED:
                    pb.status=STATUS_SELECTED+STATUS_SCROLL
                elif pb.status==STATUS_SELECTED+STATUS_SCROLL:
                    pb.status=STATUS_SELECTED
            else:
                if pb.status==STATUS_SCROLL:
                    pb.status=STATUS_ORDINARY
                elif pb.status==STATUS_SELECTED+STATUS_SCROLL:
                    pb.status=STATUS_SELECTED
            
        self.__drawPathButtons(dc)

    def __tapDraw(self):
	dc=self.__getClientDC()
	self.__backgroundDrawing(dc)
        self.__borderDrawing(dc)

        for pb in self.pathButtons:
            if pb.index==self.scrollPos:
                pb.status==STATUS_SELECTED
            else:
                if pb.status==STATUS_SELECTED:
                    pb.status=STATUS_ORDINARY
        
        self.__drawPathButtons(dc)
 
    def getStatusIcon(self,status):
        if status==STATUS_ORDINARY:
            return self.red
        elif status==STATUS_SCROLL:
            return self.yellow
        elif status==STATUS_SELECTED:
            return self.green
        elif status==STATUS_SELECTED+STATUS_SCROLL:
            return self.green


STATUS_SELECTED=2
STATUS_ORDINARY=0
STATUS_SCROLL=1

class PathButton:
    def __init__(self,parent,x,y,text,status,index):
        self.parent=parent
        self.x0=x
        self.y0=y
        self.x1=x
        if text.startswith('#'): 
            self.text=text[1:]
        else:
            self.text=text
        self.tx=self.x0+10
        self.ty=self.y0+10
        self.status=status
        self.index=index
        self.drawlink=True
    
    def getLimits(self):
        return self.x0,self.x1,self.index

    def getWidth(self,dc):
        pts,weight=fonts.setTableEntrySelectedFont(dc)
        width=10+dc.GetTextExtent(self.text)[0]
        fonts.resetFont(dc,pts,weight)
        return width

    def draw(self,dc,xoffset=0):
        self.x0=self.x0+xoffset
        pen=wx.Pen(colours.pathBoxBorder)
        pen.SetWidth(1)
        dc.SetPen(pen)
 
        w=self.getWidth(dc)-5
        h=self.parent.GetClientSize()[1]-16
        dc.DrawRectangle(self.x0+5,self.y0+8,w, h)
        dc.GradientFillLinear(wx.Rect(self.x0+6, self.y0+8,w-1,h-1),colours.horPanelGradient1,colours.horPanelGradient2,wx.SOUTH)
        
        if self.status==STATUS_SELECTED:
            dc.SetTextForeground(colours.text_sel_tap)
            xlen=dc.GetTextExtent(self.text)[0]
            self.tx=self.x0+5+(0.5*w)-0.5*xlen
            dc.DrawText(self.text,self.tx,self.ty)
            dc.SetTextForeground('black')
            yinc=dc.GetTextExtent(self.text)[1]
        elif self.status==STATUS_SCROLL:
            pts,weight=fonts.setTableEntrySelectedFont(dc)
            xlen=dc.GetTextExtent(self.text)[0]
            self.tx=self.x0+5+(0.5*w)-0.5*xlen
            dc.DrawText(self.text,self.tx,self.ty)
            yinc=dc.GetTextExtent(self.text)[1]
            fonts.resetFont(dc,pts,weight)
        elif self.status==STATUS_SELECTED+STATUS_SCROLL:
            pts,weight=fonts.setTableEntrySelectedFont(dc)
            dc.SetTextForeground(colours.text_sel_tap)
            xlen=dc.GetTextExtent(self.text)[0]
            self.tx=self.x0+5+(0.5*w)-0.5*xlen
            dc.DrawText(self.text,self.tx,self.ty)
            dc.SetTextForeground('black')
            yinc=dc.GetTextExtent(self.text)[1]
            fonts.resetFont(dc,pts,weight)

        elif self.status==STATUS_ORDINARY:
            xlen=dc.GetTextExtent(self.text)[0]
            self.tx=self.x0+5+(0.5*w)-0.5*xlen
            dc.DrawText(self.text,self.tx,self.ty)
            yinc=dc.GetTextExtent(self.text)[1]

        dc.DrawBitmap(self.parent.getStatusIcon(self.status),self.tx+(xlen*0.5)-10,self.ty+yinc-2)
        self.x1=self.x0+xlen+10
        if self.drawlink:
            pen=wx.Pen(colours.pathBoxBorder)
            pen.SetWidth(2)
            dc.SetPen(pen)
            dc.DrawLine(self.x0+5+w,0.5*self.parent.GetClientSize()[1],self.x0+5+w+5,0.5*self.parent.GetClientSize()[1])

