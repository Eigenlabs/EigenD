
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

class ScrollPanel(wx.ScrollBar):
    def __init__(self,parent,size, listener,sizer):
        wx.ScrollBar.__init__(self,parent,-1,(0,0),(size),style=wx.SB_VERTICAL)
        self.SetScrollbar(0,15,30,15)
        self.listener=listener
        self.sizer=sizer
        self.__showing=False
        self.Bind(wx.EVT_SCROLL, self.onScroll)

    def onScroll(self,evt):
        print '***** onScroll',evt.GetPosition()
        offset=evt.GetPosition()
        if self.listener:
            self.listener.scrollbarScroll(offset)


    def set(self,position,thumbSize,maxPos):
        if thumbSize<maxPos:
            if not self.__showing:
                self.sizer.Add(self,0,wx.EXPAND)
                self.sizer.Layout()
                self.__showing=True
        else:
            if self.__showing:
                self.__showing=False
                self.sizer.Detach(self)
                self.sizer.Layout()
        print 'ScrollPanel:set',position,thumbSize, maxPos
        self.SetScrollbar(position,thumbSize,maxPos,thumbSize)



