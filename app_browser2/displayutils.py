
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
from pigui import colours,fonts,utils,drawutils

class BrowseList:
    def __init__(self,parent,offset,maxOffset,position=(0,0),margins=(0,0),width=1000,height=1000,selected=0,tapped=-1):
        self.xpos=position[0]
        self.ypos=position[1]
        self.xMargin=margins[0]
        self.yMargin=margins[1]
        self.width=width
        self.height=height
        self.parent=parent
        self.offset=offset
        self.maxOffset=maxOffset
        self.itemList=[]
        self.auxDict={}
        self.highlight=[]
        self.selected=selected
        self.__tapped=-1
        self.__amber_sel=-1
        self.__tappedItem=-1
        self.lineSpacing=0
        self.gridlines=[]

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
        imageName=resource.find_release_resource('app_browser2','folder.png')
        if imageName:
            image=wx.Image(imageName,wx.BITMAP_TYPE_PNG)
            self.folder=image.ConvertToBitmap()

    def isTapped(self):
        return not self.__tapped==-1

    def setOffset(self,offset):
        self.offset=offset

    def setSelected(self,selected):
        self.selected=selected

    def setAmber(self,amber_sel):
        self.__amber_sel=amber_sel

    def setTapped(self,tapped):
        self.__tapped=tapped
        print 'BrowseList self.__tapped', self.__tapped

    def getLineSpacing(self,dc):
        return 1.5*dc.GetTextExtent('0')[1]

    def getNumItemsDisplayed(self,dc):
        lineSpacing=self.getLineSpacing(dc)

        numItems=0
        ypos=self.ypos+self.yMargin
        while ypos<self.height:
            numItems=numItems+1
            ypos=ypos+lineSpacing
        return numItems

    def setFileInfo(self,itemList):
        self.itemList=itemList

    def setAuxInfo(self,auxDict):
        self.auxDict=auxDict

    def setHighlights(self,linelist):
        self.highlight=linelist 

    def processDescription(self,items):
        desc=''
        if len(items)>1:
            desc =items[1]
        return [desc]

    def processName(self,items):
        name=''
        if len(items)>2:
           name=items[2]
        return name
    
    def __setTextForeground(self,dc,highlight,current,tapped):
        if highlight or tapped:
            dc.SetTextForeground(colours.text_sel_tap)
        elif current:
            dc.SetTextForeground(colours.text_sel)
            
    def renderDescription(self,desc,xpos,ypos,dc,highlight=False,current=False,tapped=False):
        self.__setTextForeground(dc,highlight,current,tapped)
        if desc.startswith('#'): desc=desc[1:]
        dc.DrawText(desc,xpos,ypos)
        dc.SetTextForeground('black')

    def renderCount(self,count,x,y,dc,highlight=False,current=False,tapped=False):
        self.__setTextForeground(dc,highlight,current,tapped)
        dc.DrawText(str(count),x,y)
        dc.SetTextForeground('black')
    
    def renderName(self,name,xpos,ypos,dc,highlight=False,current=False,tapped=False):
        if not name is None:
            if name.startswith('!'):
                name=name[1:]
            if name!='None':
                self.__setTextForeground(dc,highlight,current,tapped)
                dc.DrawText(name,xpos,ypos)
                dc.SetTextForeground('black')

    def __truncate(self,dc,s,max):
        while dc.GetTextExtent(s)[0]>max:
            s=s[:-4]+'...'        
        return s
                               
    def drawEntry(self,dc,item,led,xpos,ypos,desc,name,highlight=False,current=False,tapped=False):
       if str(item[0]).startswith('!control'):
           self.__drawControlEntry(dc,item,led,xpos,ypos,desc,name,highlight=highlight,current=current,tapped=tapped)
       else:
           dc.DrawBitmap(led,xpos+self.count_dx,ypos)
           if str(item[0]).startswith('!collection'):
               dc.DrawBitmap(self.folder,xpos+self.count_dx+self.led_dx,ypos)
           self.__drawFileEntry(dc,item,led,xpos,ypos,desc,name,highlight=highlight,current=current,tapped=tapped)

    def __drawFileEntry(self,dc,item,led,xpos,ypos,desc,name,highlight=False,current=False,tapped=False):
       self.renderCount(self.count,xpos,ypos,dc,highlight=highlight,current=current,tapped=tapped)
       t_desc=desc[0]
       maxlen=self.nameOffset-(self.descOffset+2)
       if dc.GetTextExtent(t_desc)[0]>maxlen:
           self.parent.truncatedValues.append((xpos+self.descOffset,xpos+self.descOffset+maxlen,ypos,ypos+self.lineSpacing,t_desc))
           t_desc=self.__truncate(dc,t_desc,maxlen)
       self.renderDescription(t_desc,xpos+self.descOffset+2,ypos,dc,highlight=highlight,current=current,tapped=tapped)
       self.renderName(name,xpos+self.nameOffset,ypos,dc,highlight=highlight,current=current,tapped=tapped)

       self.gridlines.append(Line(xpos+self.count_dx-2,ypos,xpos+self.count_dx-2,ypos+self.lineSpacing))
       self.gridlines.append(Line(xpos+self.count_dx+self.led_dx,ypos,xpos+self.count_dx+self.led_dx,ypos+self.lineSpacing))
       self.gridlines.append(Line(xpos+self.descOffset,ypos,xpos+self.descOffset,ypos+self.lineSpacing))
       self.gridlines.append(Line(xpos+self.nameOffset,ypos,xpos+self.nameOffset,ypos+self.lineSpacing))
    
    def __drawControlEntry(self,dc,item,led,xpos,ypos,desc,name,highlight=False,current=False,tapped=False):
       dc.DrawBitmap(self.folder,xpos+self.count_dx-5,ypos)
       self.renderCount(self.count,xpos,ypos,dc,highlight=highlight,current=current,tapped=tapped)
       self.renderDescription(desc[0],xpos+self.count_dx+self.led_dx+2,ypos,dc,highlight=highlight,current=current,tapped=tapped)
       self.gridlines.append(Line(xpos+self.count_dx-2,ypos,xpos+self.count_dx-2,ypos+self.lineSpacing))
       self.gridlines.append(Line(xpos+self.count_dx+self.led_dx,ypos,xpos+self.count_dx+self.led_dx,ypos+self.lineSpacing))
       self.gridlines.append(Line(xpos+self.nameOffset,ypos,xpos+self.nameOffset,ypos+self.lineSpacing))

    def __setupDimensions(self,dc):
        self.count_dx=dc.GetTextExtent('99999')[0]
        self.desc_dx=max(dc.GetTextExtent('012345678901234567890123456789')[0],0.5*self.width)
        self.led_dx=20
        self.folder_dx=30
        self.count=self.offset
        self.lineSpacing=self.getLineSpacing(dc)
        self.numItems=0
       
        ypos=self.ypos+self.yMargin
        while ypos<=self.height:
           self.numItems=self.numItems+1
           ypos=ypos+self.lineSpacing
           
        if self.count==self.maxOffset:
            self.count=self.count-1

        self.descOffset=max(self.count_dx+self.led_dx+self.folder_dx,self.width*0.2)
        
        # XXX icon not shown yet
        self.iconOffset=self.width*0.75
        self.nameOffset=self.descOffset+self.desc_dx+10

    def __getHighlightLed(self,highlight):
        if highlight:
            led=self.green
        else:
            led=self.red
        return led

    def __getAmberLed(self,amber):
       if amber:
           led=self.yellow
       else:
           led=self.red
       return led

    def __getStatus(self,i,item):
       scroll_selection=False
       tap_selection=False
       highlight=False
       amber_selection=False

       if item[0] in self.highlight:
            highlight=True
       if i==(self.selected)-self.offset:
           scroll_selection=True
       if i==(self.__tapped)-self.offset:
           tap_selection=True
       if i==(self.__amber_sel)-self.offset:
           amber_selection=True
       return (highlight,scroll_selection,tap_selection,amber_selection)
   
    def draw(self,dc):
#	drawutils.setPenColour(dc,colours.borderGradient1)
#	drawutils.setBrushColour(dc,colours.borderGradient1)
#       dc.DrawRectangle(0,0,self.width,self.height)

        self.gridlines=[]
        self.parent.truncatedValues=[]
        self.__setupDimensions(dc)
        xpos=self.xpos+self.xMargin
        ypos=self.ypos+self.yMargin
        drawutils.setPenColour(dc,'black')

        if self.maxOffset>0:
            for i in range(self.numItems): 
               self.count=self.count+1 
               if self.count>self.maxOffset:
                    break
               rectHeight=min((self.lineSpacing-1),(self.height-(ypos-1)))
	       rectHeight=rectHeight+1

               if self.itemList:
                   if i <len(self.itemList):

                       item=self.itemList[i]
                       desc=self.processDescription(item)
                       name=self.processName(item)

                       highlight,scroll_selection,tap_selection,amber_selection=self.__getStatus(i,item)

                       drawutils.setPenColour(dc,wx.Colour(0,0,0))
                       drawutils.setBrushColour(dc,wx.Colour(0,0,0))
                       rect=wx.Rect(1,ypos-1,self.width,rectHeight)
                       if tap_selection:
                           if scroll_selection:
                               dc.GradientFillLinear(rect,colours.bg_sel_tap,colours.bg_sel_tap,wx.SOUTH)
                               pts,weight=fonts.setTableEntrySelectedFont(dc)
                               self.drawEntry(dc,item,self.__getHighlightLed(highlight),xpos,ypos,desc,name,current=True,tapped=True)
                               fonts.resetFont(dc,pts,weight)
                           else:
                               dc.GradientFillLinear(rect,colours.bg_sel_tap,colours.bg_sel_tap,wx.SOUTH)
                               pts,weight=fonts.setTableEntrySelectedFont(dc)
                               self.drawEntry(dc,item,self.__getHighlightLed(highlight),xpos,ypos,desc,name,tapped=True)
                               fonts.resetFont(dc,pts,weight)
     
                       elif highlight:
                           if scroll_selection:
                               dc.GradientFillLinear(rect,colours.bg_selGradient1,colours.bg_selGradient2,wx.SOUTH)
                               pts,weight=fonts.setTableEntrySelectedFont(dc)
                               self.drawEntry(dc,item,self.green,xpos,ypos,desc,name,current=True,highlight=True)
                               fonts.resetFont(dc,pts,weight)
                           else:
                               dc.GradientFillLinear(rect,colours.borderGradient1,colours.borderGradient2,wx.SOUTH)
                               self.drawEntry(dc,item,self.green,xpos,ypos,desc,name,highlight=True)
     
                       elif scroll_selection:
                           dc.GradientFillLinear(rect,colours.bg_selGradient1,colours.bg_selGradient2,wx.SOUTH)
                           pts,weight=fonts.setTableEntrySelectedFont(dc)
                           self.drawEntry(dc,item,self.__getAmberLed(amber_selection),xpos,ypos,desc,name,current=True)
                           fonts.resetFont(dc,pts,weight)
                       
                       else: 
                           dc.GradientFillLinear(rect,colours.borderGradient1,colours.borderGradient2,wx.SOUTH)
                           self.drawEntry(dc,item,self.__getAmberLed(amber_selection),xpos,ypos,desc,name)
 
               ypos=ypos+self.lineSpacing

               drawutils.setPenColour(dc,colours.browser_gridlines)
               for l in self.gridlines:
                    l.draw(dc)
               drawutils.setPenColour(dc,'black')


class Line:
    def __init__(self,x1,y1,x2,y2):
        self.x1=x1
        self.x2=x2
        self.y1=y1
        self.y2=y2

    def setXOffset(self,offset):
        self.x1=self.x1-offset
        self.x2=self.x2-offset

    def draw(self,dc):
        dc.DrawLine(self.x1,self.y1,self.x2,self.y2)


