
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
from app_browser2 import borders,list_panel,info_panel,path_panel,belcanto_panel

class MainFrame(wx.Frame):
    def __init__(self,title,agent,size):
        wx.Frame.__init__(self,None,-1,title,(5,25),size,style=wx.DEFAULT_FRAME_STYLE)
        self.agent=agent

        self.horBox1=wx.BoxSizer(wx.HORIZONTAL)
        self.horBox2=wx.BoxSizer(wx.HORIZONTAL)
        self.vertBox=wx.BoxSizer(wx.VERTICAL)

        self.__calcSizes(size)
        self.SetMinSize((700,700))
        self.titlePanel=borders.TopPanel(self,self.LHTitleSize,agent)

        self.upperLeftSidePanel=borders.LeftSidePanel(self,self.LHPanelSize)
        self.upperRightSidePanel=borders.RightSidePanel(self,self.RHPanelSize)
        self.lowerLeftSidePanel=borders.LowerLeftSidePanel(self,self.LHPanelSize)
        self.lowerRightSidePanel=borders.LowerRightSidePanel(self,self.RHPanelSize)
        
        self.bottomPanel=borders.BottomPanel(self,self.bottomPanelSize)
        self.horDivPanel=borders.HorDivPanel(self,self.horDivPanelSize)
        self.vertDivPanel=borders.VertDivPanel(self,self.vertDivPanelSize)
        self.vertDivPanel2=borders.VertDivPanel(self,self.vertDivPanelSize)
        self.view=list_panel.ListPanel(self,self.viewSize,agent)
        self.pathPanel=path_panel.PathPanel(self, self.pathPanelSize,agent)
        self.infoPanel=info_panel.InfoPanel(self, self.infoPanelSize,agent)
        self.belcantoPanel=belcanto_panel.BelcantoPanel(self, self.belcantoPanelSize,agent)

        self.titlePanel.SetMinSize((300,30))
        self.view.SetMinSize((300,400))
        self.infoPanel.SetMinSize((300,400))

        self.Bind(wx.EVT_CLOSE, self.OnCloseWindow)

        self.layout()
        self.Bind(wx.EVT_SIZE, self.OnSize)

    def OnSize(self,evt):
        self.agent.doSize()
        evt.Skip()

    def updateStatus(self,text):
        if text=='Language agent connected':
            text='Scroll keys connected'
        elif text=='Language agent disconnected':
            text='Scroll keys disconnected'
        elif text=='No language agent connected':
            text='Scroll keys not connected'
        self.bottomPanel.updateStatus(text)

    def __calcSizes(self,size):
        xsize=size[0]
        ysize=size[1]
        self.viewSize=(20,20)
        self.LHTitleSize=(10,30)
        self.LHPanelSize=(10,10)
        self.RHPanelSize=(10,10)
        self.bottomPanelSize=(10,30)
        self.horDivPanelSize=(10,30)
        self.vertDivPanelSize=(30,10)
        self.pathPanelSize=(10,50)
        self.infoPanelSize=(20,20)
        self.belcantoPanelSize=(20,20)

    def layout(self):
        self.border_layout()

    def border_layout(self):
        self.SetTitle('EigenBrowser')
        self.horBox1.Clear()
        self.horBox1.Add(self.upperLeftSidePanel,0,wx.EXPAND)
        self.horBox1.Add(self.view,1,wx.EXPAND)
        self.horBox1.Add(self.vertDivPanel,0,wx.EXPAND)
        self.horBox1.Add(self.infoPanel,1,wx.EXPAND)
        self.horBox1.Add(self.upperRightSidePanel,0,wx.EXPAND)

        self.horBox2.Clear()
        self.horBox2.Add(self.lowerLeftSidePanel,0,wx.EXPAND)
        self.horBox2.Add(self.pathPanel,1,wx.EXPAND)
        self.horBox2.Add(self.vertDivPanel2,0,wx.EXPAND)
        self.horBox2.Add(self.belcantoPanel,1,wx.EXPAND)
        self.horBox2.Add(self.lowerRightSidePanel,0,wx.EXPAND)
 
        self.vertBox.Clear()
        self.vertBox.Add(self.titlePanel,0,wx.EXPAND)
        self.vertBox.Add(self.horBox1,1,wx.EXPAND)
        self.vertBox.Add(self.horDivPanel,0,wx.EXPAND)
        self.vertBox.Add(self.horBox2,0,wx.EXPAND)
        self.vertBox.Add(self.bottomPanel,0,wx.EXPAND)
        
        self.SetSizer(self.vertBox)
        self.Layout()

    def OnCloseWindow(self,evt):
        self.Destroy()
    
    def getViewPanel(self):
        if self.view:
            return self.view

    def getPathPanel(self):
        if self.pathPanel:
            return self.pathPanel
    
    def getInfoPanel(self):
        if self.infoPanel:
            return self.infoPanel

