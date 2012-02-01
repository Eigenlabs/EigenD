
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
from pigui import colours,fonts,drawutils

validnotes=(1,2,3,4,5,6,7,8)
ERROR_COLOUR=1
DEFAULT_COLOUR=0

class staveDrawingManager:
    def __init__(self,noteColour=colours.staff,lineColour=colours.staffLines):
        self.staves=[]
        self.noteColour=noteColour
        self.lineColour=lineColour

    def addStave(self,stave):
        self.staves.append(stave)

    def addStaves(self,staves):
        self.staves.extend(staves)

    def clear(self):
        self.staves=[]

    def scrolldraw(self,dc,xOffset=0,yOffset=0,xRange=(-10000,10000),yRange=(-10000,10000)):

        drawutils.setPenColour(dc,self.lineColour)
        drawutils.setPenWidth(dc,1)
        for stave in self.staves:
            for item in stave.lines:
                item.setXOffset(xOffset)
                item.setYOffset(yOffset)
                if xRange:
                    item.setXRange(xRange)
                item.m_draw(dc)


        drawutils.setBrushColour(dc,self.noteColour)
        drawutils.setPenColour(dc,self.noteColour)
 
        for stave in self.staves:
            for item in stave.circles:
                item.setXOffset(xOffset)
                item.setYOffset(yOffset)
                if xRange:
                    item.setXRange(xRange)
                item.m_draw(dc)
            
        for stave in self.staves:
            for item in stave.text:
                item.setXOffset(xOffset)
                item.setYOffset(yOffset)
                if xRange:
                    item.setXRange(xRange)
                item.m_draw(dc)
            
        pts,weight=fonts.setStaffNumbersFont(dc)
        for stave in self.staves:
            for item in stave.numbers:
                item.setXOffset(xOffset)
                item.setYOffset(yOffset)
                if xRange:
                    item.setXRange(xRange)
                item.m_draw(dc)
        fonts.resetFont(dc,pts,weight)

    def draw(self,dc):
        for stave in self.staves:
            stave.draw(dc)

    
class stave:
    def __init__(self,origin,height,width=None,words=[],notes=[],margins=(0,0),translation=True,parent=None,numbers=True,fitInWindow=False,interactive=False,matchesNotes=False):
        self.margins=(margins[0]*height,margins[1]*height)
        self.translation=translation
        self.parent=parent
        self.lineSpacing=0.2*(height-2*margins[1])
        self.noteRadius=int(0.5*self.lineSpacing)
        self.noteSpacing=5*self.noteRadius
        self.tailLength=7*self.noteRadius
        self.origin=(int(origin[0]+self.margins[0]),int(origin[1]+self.margins[1]))
        self.words=words
        self.notes=notes
        self.showNumbers=numbers
        self.size=(width,height)
        self.xpos=0
        self.lineWidth=1
        self.fitInWindow=fitInWindow
        self.colour=colours.defaultStaffLines
        self.status=''
        self.staveColour=colours.defaultStaffLines
        self.textColour=colours.defaultStaffLines
        self.matchesNotes = matchesNotes

        self.interactive=interactive
        self.oldXOffset=0
        self.oldYOffset=0
        self.lines=[]
        self.numbers=[]
        self.text=[]
        self.circles=[]
   
    def setColour(self,colour,staveColour=None,textColour=None):
        self.colour=colour
        if staveColour:
            self.staveColour=staveColour
        else:
            self.staveColour=self.colour
        if textColour:
            self.textColour=textColour
        else:
            self.textColour=colour

    def setStatus(self,status):
        self.status=status

    def setOrigin(self,origin):
        self.origin=(int(origin[0]+self.margins[0]),int(origin[1]+self.margins[1]))

    def __calcWidth(self,dc):
        width=2*self.margins[0]
        for word in self.words:
            if word[0]=='' and word[1][0]=='!':
                word=(word[1],word[1][1:])
            width=width+self.getWordWidth(word,dc)
        for note in self.notes:
            width=width+self.noteSpacing
            
        return width-self.noteSpacing

    def addWord(self,word):
        self.words.append(word)    

    def popWord(self):
        if self.words:
            return self.words.pop()

    def clear(self):
        self.words=[]
    
    def getWordWidth(self,word,dc):
        musicWidth=((len(word[0])-1)*self.noteSpacing)+self.noteSpacing
        if self.translation:
            textWidth=dc.GetTextExtent(word[1])[0]+self.noteSpacing-self.noteRadius
        else:
            textWidth=0
        return max(musicWidth,textWidth)
    
    def getOverallLength(self,dc):
        return self.__calcWidth(dc)
    
    def getOverallHeight(self,dc):
        height=10*self.noteRadius
        if self.translation:
            tHeight=1.25*dc.GetTextExtent('lpL')[1]
            height=height+tHeight
        if self.showNumbers:
            nHeight=0.75*dc.GetTextExtent('A')[1]
            height=height+nHeight
        return height
     
    def getNoNumberHeight(self,dc):
        height=10*self.noteRadius
        if self.translation:
            tHeight=dc.GetTextExtent('A')[1]
            height=height+tHeight
        return height

    def getStaveHeight(self,dc):
        return 8*self.noteRadius
  
    def draw(self,dc,lineWidth=1,xOffset=0,yOffset=0,yRange=(-10000,10000)):
        self.lines=[]
        self.text=[]
        self.circles=[]
        self.numbers=[]

        dxOffset=xOffset-self.oldXOffset
        self.oldXOffset=xOffset
        self.origin=(self.origin[0]-dxOffset,self.origin[1])
        
        dyOffset=yOffset-self.oldYOffset
        self.oldYOffset=yOffset
        self.origin=(self.origin[0],self.origin[1]-dyOffset)
        if (self.origin[1]<yRange[0]) or (self.origin[1]>yRange[1]):
            return

        self.lineWidth=lineWidth
        if self.interactive or self.words or self.notes:
            self.__drawStave(dc)
        self.xpos=self.origin[0]+0.5*self.noteSpacing

        if self.words:
            self.__drawWords(dc)
        if self.notes:
            self.__drawNotes(dc)
        
        self.__drawImpl(dc)

    def scrolldraw(self,dc,xOffset=0,yOffset=0,xRange=(-50,1000)):
        self.__drawImpl(dc,xOffset,yOffset,xRange)

    def __drawImpl(self,dc,xOffset=0,yOffset=0,xRange=None):
        
        for item in self.lines:
            item.setXOffset(xOffset)
            item.setYOffset(yOffset)
            if xRange:
                item.setXRange(xRange)
            item.draw(dc)

        for item in self.circles:
            item.setXOffset(xOffset)
            item.setYOffset(yOffset)
            if xRange:
                item.setXRange(xRange)
            item.draw(dc)
        
        for item in self.text:
            item.setXOffset(xOffset)
            item.setYOffset(yOffset)
            if xRange:
                item.setXRange(xRange)
            item.draw(dc)
        
        for item in self.numbers:
            item.setXOffset(xOffset)
            item.setYOffset(yOffset)
            if xRange:
                item.setXRange(xRange)
            item.draw(dc)
    
    def __drawStave(self,dc):
        penColour=self.staveColour 

        if not self.size[0]:
            width=self.__calcWidth(dc)
            height=self.size[1]
            self.size=(width,height)
       
        spacing=2*self.noteRadius
        ypos=self.origin[1]
        if self.fitInWindow:
            self.lines.append(stavelines(self.origin[0],ypos,self.parent.GetSize()[0],ypos,spacing,colour=penColour,penWidth=self.lineWidth))
        else:
            self.lines.append(stavelines(self.origin[0],ypos,self.origin[0]+self.size[0],ypos,spacing,colour=penColour,penWidth=self.lineWidth))
        if self.status:
            self.text.append(stavetext(self.status,self.origin[0]+self.size[0]+10,ypos+2*spacing,colour=self.__getStatusColour()))
    def __getStatusColour(self):
        if self.status:
            if self.status=='ok':
                return colours.staff
            elif self.status=='failed':
                return colours.error
        return colours.staff

    def __drawspace(self,xpos,dc):
        col=drawutils.getPenColour(dc)
        penColour=drawutils.setPenColour(dc,drawutils.getBackgroundColour(dc))
        spacing=2*self.noteRadius
        ypos=self.origin[1]
        self.lines.append(stavelines(xpos,ypos,xpos+self.noteSpacing,ypos,spacing,colour=penColour))

    def __drawWords(self,dc):
        wordpos=0
        groupSpacing=self.noteSpacing
        colour=self.staveColour 
        y1=self.origin[1]-1.5*self.lineSpacing
        dx=self.noteRadius+0.5*self.noteSpacing
        for word in self.words:
           if word[0]=='' and word[1][0]=='!':
               word=(word[1],word[1][1:])

           if word[1]=='***':
                colour=colours.error
           else:
                colour=self.staveColour

           wordpos=self.xpos-self.noteRadius
           if word[0]==' ' and word[1]==' ':
               self.__drawspace(self.xpos,dc)
               self.xpos=self.xpos+self.noteSpacing
           else:
               for x in range(len(word[0])):
                  note=word[0][x]
                  if not note=='!':
                      if self.__isValid(note):
                          self.__drawNote(note,self.xpos,dc,colour)
                      else:
                          self.lines.append(staveline(self.xpos-self.noteRadius,y1,self.xpos+dx,y1,colour=colour))
                      self.xpos=self.xpos+self.noteSpacing

               if self.translation:
                   self.text.append(stavetext(word[1],wordpos,self.origin[1]+10*self.noteRadius,colour=colour))
                   nextWordPos=wordpos+dc.GetTextExtent(word[1])[0]
                   if nextWordPos>self.xpos:
                        self.xpos=nextWordPos
              
           self.xpos=self.xpos+groupSpacing
        

    def __drawNotes(self,dc):
        if self.matchesNotes:
            colour=self.staveColour
        else:
            colour=colours.error
        for note in self.notes:
            if self.__isValid(note):
                self.__drawNote(note,self.xpos,dc,colour)
                self.xpos=self.xpos+self.noteSpacing

    def __drawNote(self,note,xpos,dc,colour):
        penColour=colour
        brushColour=colour
        ypos = self.origin[1]+(8-(int(note)-1))*self.noteRadius 

        if int(note)>4:
            self.circles.append(stavecircle(xpos,ypos,self.noteRadius,colour=brushColour))
            self.lines.append(staveline((xpos-self.noteRadius),ypos,(xpos-self.noteRadius),ypos+self.tailLength,colour=penColour,penWidth=self.lineWidth))
        else:
            self.circles.append(stavecircle(xpos,ypos,self.noteRadius,colour=brushColour))
            self.lines.append(staveline((xpos+self.noteRadius)-1,ypos,(xpos+self.noteRadius)-1,ypos-self.tailLength,colour=penColour,penWidth=self.lineWidth))

        if self.showNumbers:
            pts,weight=fonts.setStaffNumbersFont(dc)
            y=self.origin[1]-dc.GetTextExtent(note)[1] 
            x=self.xpos-1.3*self.noteRadius
            self.numbers.append(stavenumber(note,x,y,colour=colour))
            fonts.resetFont(dc,pts,weight)

    def __isValid(self,note):
        try:
            return int(note) in validnotes
        except:
                return False
class stavenumber:
    def __init__(self,text,x,y,colour=wx.Colour(0,0,0)):
        self.x=x
        self.y=y
        self.text=text
        self.xRange=(-10000,10000)
        self.yRange=(-10000,10000)
        self.colour=colour

    def setXOffset(self,offset):
        self.x=self.x-offset

    def setXRange(self,xRange):
        self.xRange=xRange

    def setYRange(self,yRange):
        self.yRange=yRange

    def setYOffset(self,offset):
        self.y=self.y-offset

    def __inXRange(self):
        return self.x>self.xRange[0] and self.x<self.xRange[1] 

    def __inYRange(self):
        return self.y>self.yRange[0] and self.y<self.yRange[1] 
    
    def draw(self,dc):
        pts,weight=fonts.setStaffNumbersFont(dc)
        dc.SetTextForeground(self.colour)
        if self.__inXRange() and self.__inYRange():
            dc.DrawText(self.text,self.x,self.y)
        fonts.resetFont(dc,pts,weight)

    def m_draw(self,dc):
        if self.__inXRange() and self.__inYRange():
            dc.DrawText(self.text,self.x,self.y)

class stavetext:
    def __init__(self,text,x,y,colour=wx.Colour(0,0,0)):
        self.x=x
        self.y=y
        self.text=text
        self.xRange=(-10000,10000)
        self.yRange=(-10000,10000)
        self.colour=colour

    def setXOffset(self,offset):
        self.x=self.x-offset

    def setYOffset(self,offset):
        self.y=self.y-offset
     
    def setXRange(self,xRange):
        self.xRange=xRange
    
    def setYRange(self,yRange):
        self.yRange=yRange

    def __inXRange(self):
        return self.x>self.xRange[0] and self.x<self.xRange[1] 

    def __inYRange(self):
        return self.y>self.yRange[0] and self.y<self.yRange[1] 
   
    def draw(self,dc):
        dc.SetTextForeground(self.colour)
        if self.__inXRange() and self.__inYRange():
            dc.DrawText(self.text,self.x,self.y)
    
    def m_draw(self,dc):
        if self.__inXRange() and self.__inYRange():
            dc.DrawText(self.text,self.x,self.y)


class stavecircle:
    def __init__(self,x,y,r,colour=wx.Colour(0,0,0)):
        self.x=x
        self.y=y
        self.r=r
        self.colour=colour
        self.xRange=(-10000,10000)
        self.yRange=(-10000,10000)
    
    def setXOffset(self,offset):
        self.x=self.x-offset

    def setYOffset(self,offset):
        self.y=self.y-offset

    def setXRange(self,xRange):
        self.xRange=xRange
     
    def setYRange(self,yRange):
        self.yRange=yRange

    def __inXRange(self):
        return self.x>self.xRange[0] and self.x<self.xRange[1] 

    def __inYRange(self):
        return self.y>self.yRange[0] and self.y<self.yRange[1] 
 
    def draw(self,dc):
        drawutils.setBrushColour(dc,self.colour)
        drawutils.setPenColour(dc,self.colour)
        if self.__inXRange() and self.__inYRange():
            dc.DrawCircle(self.x,self.y,self.r)

    def m_draw(self,dc):
        if self.__inXRange() and self.__inYRange():
            dc.DrawCircle(self.x,self.y,self.r)


class staveline:
    def __init__(self,x1,y1,x2,y2,colour=wx.Colour(0,0,0),penWidth=1):
        self.x1=x1
        self.x2=x2
        self.y1=y1
        self.y2=y2
        self.colour=colour
        self.penWidth=penWidth
        self.xRange=(-10000,10000)
        self.yRange=(-10000,10000)

    def setXOffset(self,offset):
        self.x1=self.x1-offset
        self.x2=self.x2-offset

    def setYOffset(self,offset):
        self.y1=self.y1-offset
        self.y2=self.y2-offset

    def setXRange(self,xRange):
        self.xRange=xRange
     
    def setYRange(self,yRange):
        self.yRange=yRange

    def __inXRange(self):
        return self.x2>self.xRange[0] and self.x1<self.xRange[1] 

    def __inYRange(self):
        return self.y2>self.yRange[0] and self.y1<self.yRange[1] 
 
    def draw(self,dc):
        drawutils.setPenColour(dc,self.colour)
        drawutils.setPenWidth(dc,self.penWidth)
        if self.__inXRange() and self.__inYRange():
            dc.DrawLine(self.x1,self.y1,self.x2,self.y2)
        drawutils.setPenWidth(dc,1)

    def m_draw(self,dc):
        if self.__inXRange() and self.__inYRange():
            dc.DrawLine(self.x1,self.y1,self.x2,self.y2)

class stavelines:
    def __init__(self,x1,y1,x2,y2,spacing,colour=wx.Colour(0,0,0),penWidth=1):
        self.x1=x1
        self.x2=x2
        self.y1=y1
        self.y2=y2
        self.spacing=spacing
        self.colour=colour
        self.penWidth=penWidth
        self.xRange=(-10000,10000)
        self.yRange=(-10000,10000)

    def setXOffset(self,offset):
        self.x1=self.x1-offset
        self.x2=self.x2-offset

    def setYOffset(self,offset):
        self.y1=self.y1-offset
        self.y2=self.y2-offset

    def setXRange(self,xRange):
        self.xRange=xRange
     
    def setYRange(self,yRange):
        self.yRange=yRange

    def __inXRange(self):
        return self.x2>self.xRange[0] and self.x1<self.xRange[1] 

    def __inYRange(self):
        return self.y2>self.yRange[0] and self.y1<self.yRange[1] 
 
    def draw(self,dc):
        drawutils.setPenColour(dc,self.colour)
        drawutils.setPenWidth(dc,self.penWidth)
        if self.__inXRange() and self.__inYRange():
            for line in range(5):
                dc.DrawLine(self.x1,self.y1+(line*self.spacing),self.x2,self.y2+(line*self.spacing))
        drawutils.setPenWidth(dc,1)

    def m_draw(self,dc):
        if self.__inXRange() and self.__inYRange():
            for line in range(5):
                dc.DrawLine(self.x1,self.y1+(line*self.spacing),self.x2,self.y2+(line*self.spacing))
        
