
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

from pisession import gui
from pigui import colours,utils,drawutils,stavedrawing,language

#def getName():
#    return 'history'

class HistoryModel(language.LanguageDisplayModel): 
    def __init__(self,langmodel,listener=None,scroller=None):
        language.LanguageDisplayModel.__init__(self,langmodel)
        self.maxItems=50

        self.items=[]
        self.__listeners=[]
        self.scroller=scroller
        self.__lasti=0
        self.__maxRequested=0
        self.__lastwords=[]
        self.__laststatus=''

    def history_cleared(self):
        self.clear()

    def addHistoryListener(self,listener):
        self.__listeners.append(listener)

    def clear(self):
        self.items=[]
        self.__lasti=0
        self.__maxRequested=0
        self.__lastwords=[]
        self.__laststatus=''
        self.update()

    def history_changed(self,max):
        if self.__maxRequested==0:
            self.__maxRequested=max-1
#        print 'HistoryModel:history_changed: max=',max,'self.__maxRequested',self.__maxRequested

        # clear history if eigend restarted
        if max==0 and self.__maxRequested>0:
            self.clear()
            self.__maxRequested=-1

        for i in range(self.__maxRequested+1,max+1):
            self.get_history(i)
        self.__maxRequested=max

    def get_history(self,k):
        r=gui.defer_bg(self.langmodel.get_history,k)
        def history_ok(result):
            i=result[0]
            h=result[1]
            print 'history ok ',i,h
            status=''
            feedback=''
            if h:
                words=self.__makeWords(h[0])
                if len(h)>1:
                    status=h[1]
                if len(h)>2:
                    feedback=h[2]
                if len(h)>3:
                    speaker=h[3]
                if words:
                    if not self.__repeat(i,words,status):
                        self.__lasti=i
                        self.__lastwords=[]
                        for w in words:
                            self.__lastwords.append(w)
                        self.__laststatus=status
                        self.__input_phrase(i,words,status,feedback,speaker)
            
        def history_failed():
            print 'history failed'
        r.setCallback(history_ok).setErrback(history_failed)
    
    def __repeat(self,i,words,status):
#        print '__repeat',i,words,status
#        print 'old vals',self.__lasti,self.__lastwords,self.__laststatus
        
        if i !=self.__lasti: return False
        if status !=self.__laststatus: return False
        if len(words) !=len(self.__lastwords): return False
        for i in range(len(words)):
            if words[i][0] !=self.__lastwords[i][0]:return False
            if words[i][1] !=self.__lastwords[i][1]:return False

        return True
    
    def __makeWords(self,w):
        if w.split()[0]=='*':
            return w.split()
        words=[]
        for word in w.split():
            english='***'
            if word.startswith('!'):
                english=self.getEnglish(word[1:])
            else:
                english=word

            words.append((word,english))
        return words

    def __input_phrase(self,i,words,status,feedback,speaker):
        item=None

        if status!='':
            for xItem in self.items:
                if xItem.id ==i:
                    if xItem.status!=status:
                        item =xItem
                        print 'item found in history items',item.id
                        break
                    else:
                        return

        print '__input_phrase',i,item,words,status,feedback,speaker
        if not item:
            if words[0]=='*':
                item=HistoryItem(i,'')
            else:
                item=MusicItem(i)
            self.__appendItem(item)

        item.append(words)
        item.set_status(status)

        if feedback:
            item.set_feedback(feedback)

        if speaker:
            item.set_speaker(speaker,self)
        
        for listener in self.__listeners:
            listener.historyUpdate()
            print 'History model call to reset scroller', self.scroller
            if self.scroller:
                self.scroller.reset_v(-1)
            listener.scroll_both(0,-1)

    def __appendItem(self,item):
        #print 'HistoryModel.__appendItem',item.music,'No. items=',len(self.items),'maxItems=',self.maxItems
        if len(self.items)<self.maxItems:
            self.items.append(item)
        else:
            for i in range(self.maxItems-1):
                self.items[i]=self.items[i+1]
            self.items[self.maxItems-1]=item

    def update(self):
        for listener in self.__listeners:
            listener.historyUpdate()

class HistoryPanel(wx.Window):
    def __init__(self,parent,size,agent,style=wx.BORDER_NONE|wx.VSCROLL):
        wx.Window.__init__(self,parent,-1,size=size,style=style)
        self.SetBackgroundColour(colours.borderGradient1)
        self.agent=agent
        self.minX=0
        self.maxX=0
        self.minY=0
        self.maxY=0
        self.yScroll=-1

        self.yGap=1
        self.model=agent.historyModel
        self.redrawRequired=False
        self.Bind(wx.EVT_PAINT,self.OnPaint)
        self.Bind(wx.EVT_SCROLLWIN, self.onScroll)
        self.Bind(wx.EVT_SIZE,self.OnSize)
        self.Bind(wx.EVT_IDLE,self.OnIdle)
        self.model.addHistoryListener(self)
        self.scrollRange=0
        self.SetScrollbar(wx.VERTICAL,0,10,9)

    def OnSize(self,evt):
        print 'history onSize'
        self.redrawRequired=True
        evt.Skip()

    def updateStatus(self,str):
        pass

    def OnIdle(self,evt):
        if self.redrawRequired:
            self.historyUpdate()
            self.Refresh();
        self.redrawRequired=False

    def OnPaint(self,evt):
	dc=self.__getClientDC()
        self.doPaint(dc)
	evt.Skip()

    def __getClientDC(self):
	if self.IsDoubleBuffered():
           dc=wx.ClientDC(self)
	else:
	   dc=wx.BufferedDC(wx.ClientDC(self))
	return dc

    def historyUpdate(self):
        print 'history update'
        dc=self.__getClientDC()
        self.initDC(dc)
        self.draw(dc)

    def getHelp(self):
        return []

    def initDC(self,dc):
        dc.SetFont(self.agent.font)
        dc.Clear()

    def close(self):
        self.agent.close_server()
        self.model.close()
          
    def getName(self):
        return getName()

    def onScroll(self,evt):
        print 'history onScroll: evtPos=',evt.GetPosition()
        v=1-2*float(evt.GetPosition())/float(self.scrollRange-self.GetClientSize()[1])
        self.yScroll=v
        self.__scroll_draw()

    def draw(self,dc):
        print 'history draw'
	self.__backgroundDrawing(dc)
        self.__drawItems(dc)
        self.afterDraw(dc)
        self.updateScrollBar()

    def updateScrollBar(self):
        scrollbarpos=self.scrollRange-self.__getYScrollPos()
        scrollbarthumb=self.GetClientSize()[1]
        scrollRange=self.scrollRange
        print 'history updateScrollBar:',scrollbarpos,scrollbarthumb,scrollRange
        self.SetScrollbar(wx.VERTICAL,scrollbarpos,scrollbarthumb,scrollRange)
    
    def afterDraw(self,dc):
        self.minY=dc.MinY()
        self.maxY=dc.MaxY()
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
 
    def doPaint(self,dc):
        self.initDC(dc)
        self.draw(dc)

    def __scrollbar_draw(self,yOffset):
        dc=self.getClientDC()
        self.initDC(dc)
        self.__scrollDrawItems(dc,yOffset=yOffset)
        scrollbarpos=yOffset
        scrollbarthumb=self.GetClientSize()[1]
        scrollRange=self.scrollRange
        print 'history updateScrollBar:',scrollbarpos,scrollbarthumb,scrollRange
        self.SetScrollbar(wx.VERTICAL,scrollbarpos,scrollbarthumb,scrollRange)
        self.__borderDrawing(dc)
 
    def __scroll_draw(self):
        dc=self.__getClientDC()
        self.initDC(dc)
	self.__backgroundDrawing(dc)
        yOffset=self.__getYScrollPos()
        self.__scrollDrawItems(dc,yOffset=yOffset)
        print 'scrolldraw',yOffset
        self.__borderDrawing(dc)
        self.SetScrollbar(wx.VERTICAL,(self.scrollRange-self.GetClientSize()[1]+yOffset),self.GetClientSize()[1],self.scrollRange)

    def __scrollDrawItems(self,dc,yOffset=0):
        items=self.model.items
        if items:
            numItems=len(items)
            for i in range((numItems-1),-1,-1):
                items[i].scrollDraw(dc,self,yOffset)   

    def __drawItems(self,dc,yOffset=0):
        ypos=self.GetClientSize()[1]-10-yOffset
        r=0

        items=self.model.items
        if items:
            numItems=len(items)
            for i in range((numItems-1),-1,-1):
                items[i].preDraw(dc,self)
                h=items[i].getSize(dc,self)[1]
                ypos=ypos-(h+self.yGap)
                r=r+h+self.yGap
                items[i].draw(dc,ypos,self)
        self.scrollRange=r-self.GetClientSize()[1]
       
    def scroll_both(self,h,v):
        if v != self.yScroll:
            self.yScroll=v
            print 'history:scroll_both',v
            self.__scroll_draw()

    def __getYScrollPos(self):
        d=self.yScroll
        delta=self.GetClientSize()[1]
        yScrollPos=-(d+1)*0.5*(self.__getDelta())
        return yScrollPos

    def __getDelta(self):
        delta=self.__yMax()-self.__yMin()
        if delta<self.GetClientSize()[1]:
            delta=self.GetClientSize()[1]
        return delta

    def __yMax(self):
        return self.maxY

    def __yMin(self):
        return self.minY+self.GetSize()[1]

    def __backgroundDrawing(self,dc):
	size=self.GetClientSize()
	drawutils.setPenColour(dc,colours.borderGradient1)
	drawutils.setBrushColour(dc,colours.borderGradient1)
        dc.DrawRectangle(0,0,size[0],size[1])

class HistoryItem:
    def __init__(self,id,txt):
        self.id=id
        self.text=txt
        self.status=''
        self.xspacing=10
        self.yspacing=10
        self.displayText=[]
        self.brush=wx.Brush("WHITE")
        self.feedback=None
        self.speaker=[]
        self.drawingItems=[]

    def set_status(self,status):
        self.status=status

    def set_feedback(self,feedback):
        self.feedback=ErrorMessage(feedback)

    def set_speaker(self,speaker,vocab):
        self.speaker=[speaker]
        self.text=speaker + ':   ' + self.text

    def get_text(self):
        return self.text

    def getMaxWidth(self,parent):
        return 0.7*parent.GetSize()[0]

    def preDraw(self,dc,parent):
        pass

    def append(self,words):
        self.text=' '.join(words[1:])

    def draw(self,dc,ypos,parent):
        self.drawingItems=[]
        xpos=self.getXPos(dc,parent)

        colour=colours.normalText
        if self.status=='failed' or self.status=='err_msg':
            colour=colours.errorText
        for row in self.displayText:
            self.drawingItems.append(drawutils.textItem(row,xpos+(self.xspacing/2),ypos-dc.GetTextExtent(row)[1],colour=colour))
            ypos=ypos+dc.GetTextExtent(row)[1]

        for di in self.drawingItems:
            di.draw(dc)

    def scrollDraw(self,dc,parent,yOffset=0):
        for di in self.drawingItems:
            di.draw(dc,yOffset=yOffset)

    def getDrawingItem(self,origin,height,parent):
        return HistoryTextItem(self.text,origin[0],origin[1],self.status)

    def getStatus(self):
        return None

    def getXPos(self,dc,parent):
        return 0.05*parent.GetSize()[0]

    def getSize(self,dc,parent):
        self.displayText=[]
        maxWidth=self.getMaxWidth(parent)
        widthTolerance=0.1*maxWidth
        utils.chopString(self.text,dc,self.getMaxWidth(parent),self.displayText,widthTolerance,True)
        maxWidth=0
        for str in self.displayText:
            width=dc.GetTextExtent(str)[0]
            if width>maxWidth:
                maxWidth=width
        height=len(self.displayText)*dc.GetTextExtent(self.text)[1]
        
        return (maxWidth+self.xspacing,height+self.yspacing)

class HistoryTextItem:
    def __init__(self,text,x,y,status):
        self.text=text
        self.x=x
        self.y=y
        self.status=status
        self.oldXOffset=0

    def draw(self,dc,lineWidth=1,xOffset=0):
        dxOffset=xOffset-self.oldXOffset
        self.oldXOffset=xOffset
        if self.status=='failed' or self.status=='err_msg':
            dc.SetTextForeground(colours.errorText)
        dc.DrawText(self.text,self.x-dxOffset,self.y)
        dc.SetTextForeground(colours.normalText)

    def get_text(self):
        return self.text


class MusicItem(HistoryItem):
    def __init__(self,id,words=[]):
        HistoryItem.__init__(self,id,'')
        self.music=words
        self.staves=[]
        self.overallStaveLength=0
        self.staveheight=0

    def append(self,words):
        self.music=words

    def set_status(self,status):
        self.status=status
    
    def get_text(self):
        print 'Music item:get_text',self.music
        return 'Music Item******'

    def set_speaker(self,speaker,vocab):
        self.speaker=[]
        if speaker:
            for w in speaker.split():
                m=vocab.getMusic(w)
                self.speaker.append((m,w))
            self.speaker.append((' ',' '))

    def preDraw(self,dc,parent):
        self.drawingItems=[]
        self.staves=[]
        self.staveheight=2*dc.GetTextExtent('0')[1]
        self.staves.append(stavedrawing.stave((0,0),self.staveheight,words=[],margins=(0.1,0.1),translation=True,matchesNotes=True))
        words=list(self.music)
        swords=[]
        if self.speaker:
            swords.extend(self.speaker)
            swords.extend(words)
            if swords:
                words=swords
     
        words.reverse()
        lineNo=0
        while words:
            self.staves[lineNo].addWord(words.pop())
            length=self.staves[lineNo].getOverallLength(dc)
            if length>0.8*parent.GetSize()[0]:
                if len(self.staves[lineNo].words)>1:
                    words.append(self.staves[lineNo].popWord())
                lineNo=lineNo+1
                self.staves.append(stavedrawing.stave((0,0),self.staveheight,words=[],margins=(0.1,0.1),translation=True,matchesNotes=True))
        if self.feedback:
            self.feedback.set_width(parent.GetSize()[0])

    def getDrawingItem(self,origin,height,parent):
        drawingItem=stavedrawing.stave(origin,height,words=self.music,parent=parent,matchesNotes=True)
        drawingItem.setStatus(self.status)
        if self.status=='message':
            drawingItem.setColour(colours.talkerText,colours.talkerStaffLines)
        elif self.status=='failed' or self.status=='err_msg':
            drawingItem.setColour(colours.error,colours.errorStaffLines)
        else:
            drawingItem.setColour(colours.staff,colours.staffLines)
        return drawingItem

    def getStatus(self):
        return self.status

    def draw(self,dc,ypos,parent):
        xIndent=25
        xpos=self.getXPos(dc,parent)
        if self.staves:
            staveHeight=self.staves[0].getStaveHeight(dc)
            height=self.staves[0].getOverallHeight(dc)
            height=height+0.3*dc.GetTextExtent('A')[1]
            origin=(xpos,ypos)

            for stave in self.staves:
                if self.status=='message':
                    stave.setColour(colours.talkerText,colours.talkerStaffLines)
                elif self.status=='failed' or self.status=='err_msg':
                    stave.setColour(colours.error,colours.errorStaffLines)
                stave.setOrigin(origin)
                origin=(xpos+xIndent,origin[1]+height)

            self.overallStaveLength=self.staves[0].getOverallLength(dc)
            
            if self.music:
                for stave in self.staves:
                    stave.draw(dc)
                if not self.status=='':
                    x=xpos+10+self.staves[self.__getNumStaves()-1].getOverallLength(dc)
                    if self.__getNumStaves()>1:
                        x=x+xIndent
                    y=self.staves[self.__getNumStaves()-1].origin[1]+0.5*staveHeight-0.5*dc.GetTextExtent(self.status)[1]

                    if self.status !='err_msg':
                        if self.status=='failed':
                            self.drawingItems.append(drawutils.textItem(self.status,x,y,colour=colours.errorText))
                        else:
                            self.drawingItems.append(drawutils.textItem(self.status,x,y,colour=colours.normalText))
                   
                if self.feedback:
                    y=self.staves[self.__getNumStaves()-1].origin[1]+self.staves[0].getOverallHeight(dc)+0.3*dc.GetTextExtent('A')[1]
                    self.feedback.set_origin(xpos,y)
                    self.feedback.set_width(parent.GetSize()[0])
                    self.feedback.draw(dc)

                for di in self.drawingItems:
                    di.draw(dc)
            
    def scrollDraw(self,dc,parent,yOffset):
        max=parent.GetSize()[1]+50
        min=-50
        for stave in self.staves:
            stave.draw(dc,yOffset=yOffset,yRange=(min,max))
        for di in self.drawingItems:
            di.draw(dc,yOffset=yOffset)
        if self.feedback:
            self.feedback.draw(dc,yOffset=yOffset)

    def __getNumStaves(self):
        if self.staves:
            return len(self.staves)
                
    def getSize(self,dc,parent):
        maxWidth=self.overallStaveLength
        height=self.staves[0].getOverallHeight(dc)
        height=height+0.3*dc.GetTextExtent('A')[1]
        height=self.__getNumStaves()*height
        if self.feedback:
            height=height+self.feedback.getHeight(dc)
        return (max(20,maxWidth+self.xspacing),height)

class ErrorMessage:
    def __init__(self,message,origin=None,width=None):
        self.message=message
        self.origin=origin
        self.width=width

    def draw(self,dc,yOffset=0):
        y=self.origin[1]
        for line in self.__getLines(dc):
           s=' '.join(line)
           dc.DrawText(s,self.origin[0],y-yOffset)
           y=y+1.2*dc.GetTextExtent(s)[1]

    def set_origin(self,x,y):
        self.origin=(x,y)

    def set_width(self,width):
        self.width=width

    def getHeight(self,dc):
        numlines=len(self.__getLines(dc))
        height=numlines*1.2*dc.GetTextExtent('A')[1]
        height=height+dc.GetTextExtent('A')[1]
        return height
    
    def __getLines(self,dc):
        lines=[]
        lines.append([])
        words=self.message.split()
        words.reverse()
        lineNo=0
        while words:
            lines[lineNo].append(words.pop())
            
            length=dc.GetTextExtent(' '.join(lines[lineNo]))[0]

            if length>0.8*self.width:
                if len(lines[lineNo])>1:
                    words.append(lines[lineNo].pop())
                lineNo=lineNo+1
                lines.append([])

        return lines 

