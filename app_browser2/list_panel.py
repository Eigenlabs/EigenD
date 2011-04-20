
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
from app_browser2 import displayutils

class ListPanel(wx.Window):
    def __init__(self,parent,size,agent,style=wx.BORDER_NONE | wx.VSCROLL):
        wx.Window.__init__(self,parent,-1,size=size,style=style)  
	self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)
        self.SetBackgroundColour(colours.borderGradient1)
        self.model=agent[8].model
        self.model.addListListener(self)
        self.Bind(wx.EVT_LEFT_DOWN,self.onLeftDown)
        self.Bind(wx.EVT_LEFT_UP,self.onLeftUp)
        self.Bind(wx.EVT_MOTION,self.onMotion)
        self.Bind(wx.EVT_MOUSEWHEEL,self.onMouseWheel)
        self.Bind(wx.EVT_PAINT, self.onPaint)
        self.Bind(wx.EVT_SCROLLWIN, self.onScroll)
        self.Bind(wx.EVT_IDLE,self.onIdle)
        self.offset=0

	# XXX
        #self.font=wx.Font(13,wx.FONTFAMILY_MODERN,wx.FONTSTYLE_NORMAL,weight=wx.FONTWEIGHT_LIGHT)

        self.font=wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        self.font.SetPointSize(13)

        self.mainFrame=parent
        self.__agent=agent
        self.yScroll=1
        #self.selection=1
        self.numColDisp=0
        self.topSelection=0
        self.__colOffset=0
        self.cl=None
        self.fl=None
        self.__history=[]
        self.refresh=True
        self.__drawRequired=False

        self.oldv=1
        self.neginc=0
        self.posinc=0
 
        self.__numItemsDisplayed=0

        self.__scrolling=False
        self.__selection_changed=False

        self.truncatedValues=[]
        self.SetToolTip(wx.ToolTip(''))

        self.timer=wx.Timer(self,1001)
        self.Bind(wx.EVT_TIMER,self.onTimer,id=1001)
        self.timer.Start(500)
        
        self.timer2=wx.Timer(self,1002)
        self.Bind(wx.EVT_TIMER,self.onTimer2,id=1002)

        self.SetScrollbar(wx.VERTICAL,0,0,0)

    def onIdle(self,evt):
        if self.__drawRequired:
            self.scrollDrawFiles()
            
    def onScroll(self,evt):
        if self.fl:
            offset=evt.GetPosition()
            self.scrollbarScroll(offset)

    def onLeftDown(self,evt):
        pass

    def onLeftUp(self,evt):
        if self.fl:
            mPos=evt.GetPositionTuple()
            y=mPos[1]
            self.mouseSelect(y)
            selection=self.offset+int(y/self.fl.lineSpacing)
            if selection==self.topSelection:
                self.__doTap()
       
    def onMotion(self,evt):
        if self.fl:
            mPos=evt.GetPositionTuple()
            self.mouseSelect(mPos[1])
            for v in self.truncatedValues:
                if mPos[0]>v[0] and mPos[0]<v[1] and mPos[1]>v[2] and mPos[1]<v[3]:
                    self.GetToolTip().SetTip(v[4])
                    return 
        
            self.GetToolTip().SetTip('')

    def onMouseWheel(self,evt):
        if self.fl:
            delta=evt.GetWheelDelta()
            rotation=evt.GetWheelRotation()
    #        print 'onMouseWheel:delta=',delta,'rotation=',rotation
            deltaOffset=rotation*delta*evt.GetLinesPerAction()
            self.wheelScroll(deltaOffset)

    def wheelScroll(self,deltaOffset):
        if self.fl:
            offset=self.offset-deltaOffset
            numFiles=self.model.numFiles+self.model.numCollections
            if offset>=0 and offset<=((numFiles+1)-self.__numItemsDisplayed):
                self.offset=offset
                self.fl.setOffset(self.offset)
                self.__drawRequired=True

    def onTimer2(self,evt):
        if self.fl:
            if self.fl.isTapped():
                self.fl.setTapped(-1)
                self.__drawRequired=True

    def onTimer(self,evt):
        #print 'onTimer','scrolling=',self.__scrolling,'selection_changed',self.__selection_changed
        self.__doTopSelection()

    def tap(self,n):
        if n==1:
            self.__doTap()

    def __doTap(self):
        if self.fl:
            #self.fl.setTapped(self.topSelection)
            #self.__drawRequired=True

            if self.__isFile(self.topSelection):
                self.fl.setTapped(self.topSelection)
                self.__drawRequired=True
                self.model.activate(self.topSelection+1-self.numColDisp)
                self.timer2.Start(500,oneShot=True)
            else:
                print '__doTap',self.topSelection, self.model.cinfo[self.topSelection]
                if self.model.cinfo[self.topSelection][1]=='Up one level':
                    self.model.back() 
                else:
                    self.model.changeDir(self.topSelection)

#            self.timer2.Start(500,oneShot=True)

    def update(self):
        print 'ListPanel:update'

	dc=self.__getClientDC()
        self.doPaint(dc)
        self.__borderDrawing(dc)
    
    def set_refresh(self):
        self.refresh=True

    def set_selected(self,selected):
        self.topSelection=selected

    def resetVScroller(self,v):
        pass
#        self.__agent.reset1(1,v)
 
    def onPaint(self,evt):
	bdc=self.__getClientDC()
        bdc.Clear()
	brush=bdc.GetBrush()
	brush.SetColour(colours.borderGradient1)
	bdc.SetBackground(brush)

        bdc.SetFont(self.font)
        self.doPaint(bdc)
        evt.Skip()

    def doPaint(self,dc):
        if self.refresh:
            self.__history=[]
	self.__backgroundDrawing(dc)
        self.doDrawing(dc)
        self.__borderDrawing(dc)

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
    
    def doDrawing(self,dc):
        print 'list_panel:doDrawing',self.model.numFiles,self.model.numCollections
        if self.model.numFiles==0 and self.model.numCollections==0:
            self.SetScrollbar(wx.VERTICAL,0,0,0)
            self.__drawIcon(dc)
            self.fl=None
        else:
            self.__doTopPanelDraw(dc)
            self.fl.draw(dc)

    def __drawIcon(self,dc):
        print 'list_panel:drawIcon'
        if self.model.icon:
            icon=utils.iconHandler(self.model.icon)
            dc.DrawBitmap(icon,(0.5*self.GetClientSize()[0])-32,20)
        else:
            print 'icon is None'

    def __doTopPanelDraw(self,dc):
        if self.model.finfo_for_new_category and self.model.finfo:
            self.__selection_changed=True
            self.model.finfo_for_new_category=False
        
        if self.topSelection>=self.model.numFiles+self.model.numCollections:
            print 'topSelection=',self.topSelection,'numFiles=',self.model.numFiles
            self.topSelection=0
#            self.resetVScroller(1)

        self.__doTopSelection(force=True)
        self.fl=self.__getDisplayList(self.model.targetName,self.offset,self.model.numFiles+self.model.numCollections)
        self.__doTopPanelData(dc)

    def __doTopPanelData(self,dc):
        offset=self.offset
        maxOffset=self.model.numFiles+self.model.numCollections
        numItemsDisplayed=self.fl.getNumItemsDisplayed(dc)
        self.__numItemsDisplayed=numItemsDisplayed
        self.SetScrollbar(wx.VERTICAL,self.offset,self.__numItemsDisplayed,maxOffset+1)
        maxDisplayed=self.offset + numItemsDisplayed
        maxDisplayed=min(maxOffset,maxDisplayed)-1
        if maxDisplayed<0:
            maxDisplayed=0

        self.fl.setFileInfo(self.__getDisplayData(offset,numItemsDisplayed))
        self.fl.setHighlights([self.model.current])

    def __getDisplayData(self,offset,displaylength):
        displaylist=[]
        displaylist.extend(self.model.cinfo)
        displaylist.extend(self.model.finfo)
        self.numColDisp=self.model.numCollections
        return displaylist[offset:]

    def __isFile(self,selection):
        return selection>= self.model.numCollections

    def __getDisplayList(self,browseType,offset,maxOffset):
       print '__getDisplayList:browseType=',browseType, maxOffset
       return displayutils.BrowseList(self,offset,maxOffset,position=(0,0),margins=(10,0),width=self.GetSize()[0],height=self.GetSize()[1],selected=self.topSelection)

    def __doTopSelection(self,force=False):
#        print '__doTopSelection: force=',force,'scrolling',self.__scrolling,'selection_changed',self.__selection_changed
        if force or((not self.__scrolling) and self.__selection_changed):
            print 'TopPanel:onTimer',self.topSelection
            
            if self.__isFile(self.topSelection):
                self.model.setSelectedFile(self.topSelection+1-self.numColDisp)
                if self.fl:
                    self.fl.setAmber(self.topSelection)
                self.__drawRequired=True
            self.__selection_changed=False
        self.__scrolling=False 

    def mouseScroll(self,v):
        #print 'mousescroll',v
        if self.fl:
#            self.resetVScroller(v)
            self.scroll_both(1,v,mouse=True)

    def scrollbarScroll(self,offset):
        if self.fl:
            self.offset=offset
            self.fl.setOffset(self.offset)
#            self.__drawRequired=True
            self.scrollDrawFiles()

    def mouseSelect(self,y):
        #print 'list mouseSelect: y=',y, 'offset=',self.offset, int(y/self.fl.lineSpacing), self.model.numFiles,self.model.numCollections
        self.__scrolling=True
        selection=self.offset+int(y/self.fl.lineSpacing)
        #print 'selection=',selection,'topSelection',self.topSelection
        if selection!=self.topSelection:
            self.__selection_changed=True
            if selection<self.model.numFiles+self.model.numCollections:
                self.topSelection=selection 
                self.fl.setSelected(self.topSelection)
                self.__drawRequired=True
                
                nf=self.model.numFiles
                nc=self.model.numCollections
                n=nf+nc
                if n==0:
                    pass
#                    self.resetVScroller(1)
                else:
                    v=-2*(float(self.topSelection)/float(self.model.numFiles+self.model.numCollections))+1
#                    self.resetVScroller(v)

    # h is roll v is yaw.
    def scroll_both(self,h,v,mouse=False):
        if self.fl:
            if h is not None: self.xScroll=h
            if v is not None: self.yScroll=v
            if abs(self.oldv-v)>0.0001:
                self.__scrolling=True
                numFiles=self.model.numFiles+self.model.numCollections
                if numFiles>5:
                    p=5
                else:
                    p=1
                
                ys=int((-0.5*(v-1))*(numFiles-p))
                selection=int((-0.5*(v-1))*(numFiles-1))
    #            print 'scroll_both',selection,self.topSelection, numFiles
                if selection!=self.topSelection:
                    self.__selection_changed=True
                self.topSelection=selection
                if self.__scrolling_down(v):
                    sel_crit=self.offset+(self.__numItemsDisplayed-2)
                    if self.topSelection>sel_crit:
                         delta=self.topSelection-sel_crit
                         self.offset=self.offset+delta
                elif self.__scrolling_up(v):
                    if (self.topSelection-self.offset)<=0: 
                        self.offset=ys
                self.oldv=v
                self.fl.setSelected(self.topSelection)
                self.fl.setOffset(self.offset)
                self.__drawRequired=True
                self.SetScrollbar(wx.VERTICAL,self.offset,self.__numItemsDisplayed,numFiles+1)

    def __scrollIncrement(self,v):
#        m=1
#        if v>0:
#            m=-1
#        v=abs(v)
#        if v<0.1:
#            inc=0
#        elif v<0.2:
#            inc=2.5*v-0.25
#        elif v<0.3:
#            inc=7.5*v-1.25
#        elif v<0.4:
#            inc=90*v-26
#        elif v<0.45:
#            inc=1800*v-710
#        else:
#            inc=8000*v-3500
#        return m*inc
#         
        m=1
        if v>0:
            m=-1
        v=abs(v)
        if v<0.1:
            inc=0
        elif v<0.2:
            inc=v-0.1
        elif v<0.3:
            inc=1.5*v-0.2
        elif v<0.4:
            inc=7.5*v-2
        elif v<0.45:
            inc=80*v-31
        elif v<0.475:
            inc=600*v-265
        elif v<0.49:
            inc=5334*v-2514
        else:
            inc=40000*v-19500
        return m*inc
     
#    # h is roll v is yaw.
#    def test_scroll_both(self,v):
#        print 'test scroll',v
#        self.__scroll2(v)

    def scroll2(self,inc):
        if self.fl:
            self.__scrolling=True
            numFiles=self.model.numFiles+self.model.numCollections
            selection=self.topSelection+inc
            if selection<0:
                selection=0
            if selection>numFiles-1:
                selection=numFiles-1
            ys=selection
            if selection!=self.topSelection:
                self.__selection_changed=True
            self.topSelection=selection


            if inc==1:
               sel_crit=self.offset+(self.__numItemsDisplayed-2)
               if self.topSelection>sel_crit:
                delta=self.topSelection-sel_crit
                self.offset=self.offset+delta
            elif inc==-1:
                if (self.topSelection-self.offset)<=0:
                    self.offset=ys

            self.fl.setSelected(self.topSelection)
            self.fl.setOffset(self.offset)
            self.__drawRequired=True
            self.SetScrollbar(wx.VERTICAL,self.offset,self.__numItemsDisplayed,numFiles+1)
 
    def scrollDrawFiles(self):
#        print 'scrollDrawFiles'
        self.__drawRequired=False
        if self.fl:
	    dc=self.__getClientDC()
	    self.__backgroundDrawing(dc)
            self.__doScrollDrawFiles(dc)
            self.__borderDrawing(dc)

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

    def __doScrollDrawFiles(self,dc):
        if self.fl:
            self.__doTopPanelData(dc)
            self.fl.draw(dc)
    
    def __scrolling_up(self,v):
        return v>self.oldv

    def __scrolling_down(self,v):
        return v<self.oldv

