
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
from pigui import colours,utils,drawutils,fonts,stavedrawing

class TextLine:
    def __init__(self,parent,text=''):
        self.readOnly=False
        self.parent=parent
        self.prompt='>'
        self.text=self.prompt+text
        self.selecting=False
        self.selectionLeft=0
        self.selectionRight=0
        self.insertionPoint=len(self.text)
        self.mouseIndex=0

    def stripPrompt(self,str):
        if str[:len(self.prompt)]==self.prompt:
            return str[len(self.prompt):]
        else:
            return str

    def resetInsertion(self):
        self.insertionPoint=len(self.text)

    def getCommandString(self):
        return self.stripPrompt(self.text)

    def clear(self):
        self.text=''

    def set_prompt(self):   
        self.text=self.prompt

    def append(self,text):
        self.text=self.text +text

    def insertText(self,text):
        if self.insertionPoint<len(self.text):
           i=self.insertionPoint
           self.text=self.text[0:i]+text+self.text[i:]
           self.insertionPoint=self.insertionPoint+len(text)
        else:
           self.text=self.text+text      
           self.insertionPoint=len(self.text)

    def onEscape(self):
        self.set_prompt()
        self.insertionPoint=len(self.text)

    def set_text(self,text):
        self.text=text

    def cut(self):
        if self.selecting:
            cut_text=''
            line=self.stripPrompt(self.text)
            if line:
                cut_text=line[self.selectionLeft-1:self.selectionRight-1]
            text_data=wx.TextDataObject(cut_text)
            if wx.TheClipboard.Open():
                wx.TheClipboard.SetData(text_data)
                wx.TheClipboard.Close()
           
            self.__cutSelection()

            self.selecting=False
#            print 'doCut',cut_text

    def __cutSelection(self):
       line=self.stripPrompt(self.text)
       self.text=self.prompt + line[:self.selectionLeft-1]+ line[self.selectionRight-1:]
       cutlen=self.selectionRight-self.selectionLeft
       if self.insertionPoint>self.selectionLeft:
            self.insertionPoint=self.insertionPoint-cutlen
       if self.insertionPoint>len(self.text):
          self.insertionPoint=len(self.text)

    def copy(self):
        if self.selecting:
            copy_text=''
            line=self.stripPrompt(self.text)
            if line:
                copy_text=line[self.selectionLeft-1:self.selectionRight-1]
            text_data=wx.TextDataObject(copy_text)
            if wx.TheClipboard.Open():
                wx.TheClipboard.SetData(text_data)
                wx.TheClipboard.Close()
            self.selecting=False
#            print 'doCopy', copy_text

    def paste(self):
#        print 'doPaste'
        text_data=wx.TextDataObject()
        if wx.TheClipboard.Open():
            success=wx.TheClipboard.GetData(text_data)
            wx.TheClipboard.Close()
        if success:
            if self.selecting:
                self.__cutSelection();
#            print 'paste success', text_data.GetText()
            self.insertText(text_data.GetText())
        self.selecting=False

    def doMouseMove(self,index):
        if index>self.mouseIndex:
            # mouse moved right
            self.__mouseRight(index)
        elif index<self.mouseIndex:
            # mouse moved left
            self.__mouseLeft(index)

    def __mouseLeft(self,index):
       if self.mouseIndex==self.selectionRight:
           if index>self.selectionLeft:
                self.selectionRight=index
           elif index <=self.selectionLeft:
                self.selectionRight=self.selectionLeft
                self.selectionLeft=index
       elif self.mouseIndex==self.selectionLeft:
            self.selectionLeft=index
            
       self.mouseIndex=index 

    def __mouseRight(self,index):
#       print '__mouseRight', index
       if self.mouseIndex==self.selectionLeft:
         if index <=self.selectionRight:
                self.selectionLeft=index
         elif index>self.selectionRight:
                self.selectionLeft=self.selectionRight
                self.selectionRight=index
       elif self.mouseIndex==self.selectionRight:
            self.selectionRight=index

       self.mouseIndex=index 

    def onLeft(self):
        if self.insertionPoint>len(self.prompt):
            self.insertionPoint=self.insertionPoint-1
        self.selecting=False

    def onLeftShift(self):
        newp=self.insertionPoint
        if self.insertionPoint>len(self.prompt):
            newp=self.insertionPoint-1

        if self.selecting:
            if self.selectionLeft==self.insertionPoint:
                self.selectionLeft=newp
            elif self.selectionRight==self.insertionPoint:
                self.selectionRight=newp
        else:
            self.selectionRight=self.insertionPoint
            self.selectionLeft=newp

        self.selecting=True
        self.insertionPoint=newp
        
    def onRight(self):
        if self.insertionPoint<len(self.text):
            self.insertionPoint=self.insertionPoint+1
        self.selecting=False

    def onRightShift(self):
        newp=self.insertionPoint
        if self.insertionPoint<len(self.text):
            newp=self.insertionPoint+1

        if self.selecting:
            if self.selectionRight==self.insertionPoint:
                self.selectionRight=newp
            elif self.selectionLeft==self.insertionPoint:
                self.selectionLeft=newp
        else:
            self.selectionLeft=self.insertionPoint
            self.selectionRight=newp

        self.selecting=True
        self.insertionPoint=newp

    def onRight(self):
        if self.insertionPoint<len(self.text):
            self.insertionPoint=self.insertionPoint+1
        self.selecting=False

    def onRightShift(self):
        newp=self.insertionPoint
        if self.insertionPoint<len(self.text):
            newp=self.insertionPoint+1

        if self.selecting:
            if self.selectionRight==self.insertionPoint:
                self.selectionRight=newp
            elif self.selectionLeft==self.insertionPoint:
                self.selectionLeft=newp
        else:
            self.selectionLeft=self.insertionPoint
            self.selectionRight=newp

        self.selecting=True
        self.insertionPoint=newp

    def onBack(self):
        if self.selecting:
            #self.doCut()
            self.cut()
        else:
            if len(self.text)>len(self.prompt):
                if self.insertionPoint<len(self.text):
                    if self.insertionPoint>len(self.prompt):
                        i=self.insertionPoint
                        self.text=self.text[0:i-1]+self.text[i:]
                        self.onLeft()
                else:
                    self.text=self.text[:-1]
                    self.insertionPoint=len(self.text)
                self.parent.drawText(wx.ClientDC(self.parent))
        self.selecting=False

 

class DisplayLine:
    def __init__(self,text,lineNo,start,end,selectionLeft,selectionRight,selecting):
        self.text=text
        self.lineNo=lineNo
        self.start=start
        self.end=end
        self.selectionLeft=selectionLeft
        self.selectionRight=selectionRight
        self.selecting=selecting
        self.__x=0
        self.__y=0
        self.__w=0
        self.__h=0

    def setExtent(self,x,y,w,h):
        self.__x=x
        self.__y=y
        self.__w=w
        self.__h=h

    def getX(self):
        return self.__x

    def getY(self):
        return self.__y

    def draw(self,dc):
        if self.selecting:
#            print 'drawSelection for line',self.lineNo
            self.drawSelection(self.selectionLeft,self.selectionRight,dc)
        dc.DrawText(self.text,self.__x,self.__y)

    def drawSelection(self,selectionLeft,selectionRight,dc):
        #rect=self.getRectangle(0,selectionLeft,selectionRight-1,dc)
        rect=self.getRectangle(selectionLeft,selectionRight-1,dc)
        if rect:
            drawutils.setPenColour(dc,colours.commandlineSelectedText)
            drawutils.setBrushColour(dc,colours.commandlineSelectedText)
            dc.DrawRectangle(rect[0],rect[1],rect[2],rect[3])

    def includes(self,index):
        return (index>=self.start) and (index<=self.end)

    def includes2(self,index):
#        print 'includes2'
        return (index>=self.start) and (index<=self.end+1)

    def lessthan(self,index):
        return (index>self.end)

    def greaterthan(self,index):
        return (index<self.start)     

    def contains(self,pos):
        mx=pos[0]
        my=pos[1]
        
        if (mx>=self.__x) and (mx<=(self.__x +self.__w)):
            if (my>=self.__y) and (my<=(self.__y+self.__h)):
#                print 'x and y in line',self.lineNo, 'range',self.start, self.end
                return True
        return False

    def getIndex(self,pos,dc):
        for i in range (len(self.text)+1):
            x=self.__x+dc.GetTextExtent(self.text[:i])[0]
            if x>=pos[0]:
                return self.lineNo,self.start+i

    def getPos(self,index,dc):
        if self.includes2(index):
            y=self.getY()
            dx=dc.GetTextExtent(self.text[0:index-self.start])[0]
            x=self.getX()+dx
            return self.lineNo,x,y

    def getRectangle(self,lIndex,rIndex,dc):
        h=self.__h
        y=self.getY()

        if self.includes(lIndex):
            if self.includes(rIndex):
                dx=dc.GetTextExtent(self.text[0:lIndex-self.start])[0]
                x=self.getX()+dx
                dx=dc.GetTextExtent(self.text[0:rIndex-self.start+1])[0]
                w=(self.getX()+dx)-x
                return x,y,w,h

            elif self.lessthan(rIndex): 
                dx=dc.GetTextExtent(self.text[0:lIndex-self.start])[0]
                x=self.getX()+dx
                w=dc.GetTextExtent(self.text[lIndex-self.start:])[0]
                return x,y,w,h

        elif self.greaterthan(lIndex):
            if self.includes(rIndex):
                x=self.getX()
                w=dc.GetTextExtent(self.text[:rIndex-self.start+1])[0]
                return x,y,w,h
            elif self.lessthan(rIndex):
                x=self.getX()
                w=dc.GetTextExtent(self.text)[0]
                return x,y,w,h


class DisplayLineManager:
    def __init__(self,parent):
        self.displayLines=[]
        self.parent=parent

        self.cursorStarted=False
        self.cursorX=0
        self.dx=0
        self.cursorY=0

    def clear(self):
        self.displayLines=[]

    def add(self,displayline):
        self.displayLines.append(displayline)

    def draw(self,dc):
        numLines=self.calcLinesToShow(dc)
        for i in range(numLines, 0,-1):
            self.displayLines[i-1].draw(dc)

    def mPos2Index(self,mPos,dc):
        for dl in self.displayLines:
           if dl.contains(mPos):
               return dl.getIndex(mPos,dc)

    def index2Pos(self,index,dc):
        for dl in self.displayLines:
            if dl.lineNo==0 and dl.includes2(index):
                return dl.getPos(index,dc)

    def load(self,dc,lines):
        lineNo=0
        for line in lines:
            start=0
            end=0
            choppedLine=[]
            utils.chopString(line.text,dc,self.parent.getMaxWidth(),choppedLine)
            tmp=[]
            for text in choppedLine:
                end=start+len(text)-1
                tmp.append(DisplayLine(text,lineNo,start,end,line.selectionLeft,line.selectionRight,line.selecting))
                start=end+1
            while tmp:
                self.add(tmp.pop())
            lineNo=lineNo+1

        xpos=5
        ypos=0

        for i in range(self.calcLinesToShow(dc), 0,-1):
            self.displayLines[i-1].setExtent(xpos,ypos,self.parent.getMaxWidth(),self.getLineHeight(dc))
            ypos=ypos+self.getLineHeight(dc)

    def calcLinesToShow(self,dc):
        h=self.parent.GetClientSize()[1] 
        textHeight=self.getLineHeight(dc)
        numLines=int(h/textHeight)
       
        if len(self.displayLines)<numLines:
            numLines=len(self.displayLines)
        return numLines

    def getLineHeight(self,dc):
        return int(1.1*dc.GetTextExtent('>')[1])
 
    def drawCursor(self,dc,insertionPoint):
        self.dx=dc.GetTextExtent('x')[0]
        r=self.index2Pos(insertionPoint,dc)
        if r:
            self.cursorStarted=True
            self.cursorX=r[1]
            self.cursorY=r[2]
            self.cursorY=self.cursorY+1.1*dc.GetTextExtent('x')[1]

        self.__doDrawCursor(dc)

    def blinkCursor(self,dc):
        self.__doDrawCursor(dc)

    def __doDrawCursor(self,dc,idle=True):
        pen=wx.Pen(colours.commandlineText)
        if idle:
            if self.parent.blink:
                pen=wx.Pen(colours.commandlineText)
            else:
                pen=wx.Pen(colours.commandlineBackground)
        pen.SetWidth(2)
        dc.SetPen(pen)
        dc.DrawLine(self.cursorX,self.cursorY,self.cursorX+self.dx,self.cursorY)

class StaffPanel(wx.Window):
    def __init__(self,parent,size,agent,style=wx.BORDER_NONE):
        wx.Window.__init__(self,parent,-1,size=size,style=style)
        self.SetBackgroundColour(colours.commandlineBackground)
        self.Bind(wx.EVT_PAINT,self.OnPaint)
        self.agent=agent
        self.model=agent.commandModel
        self.model.addCommandListener(self)
        self.font=wx.Font(13,wx.FONTFAMILY_MODERN,wx.FONTSTYLE_NORMAL,weight=wx.FONTWEIGHT_NORMAL)


    def OnPaint(self,evt):
        dc=wx.AutoBufferedPaintDC(self)
        dc.SetFont(self.font)
        dc.Clear()
        self.draw(dc)
        evt.Skip()

    def __getClientDC(self):
        if self.IsDoubleBuffered():
           dc=wx.ClientDC(self)
        else:
           dc=wx.BufferedDC(wx.ClientDC(self))
        return dc

    def __backgroundDrawing(self,dc):
        size=self.GetClientSize()
        drawutils.setPenColour(dc,colours.borderGradient1)
        drawutils.setBrushColour(dc,colours.borderGradient1)
        dc.DrawRectangle(0,0,size[0],size[1])

    def draw(self,dc):
        print 'StaffPanel draw'
        self.__backgroundDrawing(dc)
        pts,weight=fonts.setStaffFont(dc)
 
        self.height=min([self.GetSize()[1],4*dc.GetFont().GetPointSize()])
        self.setOrigin()
        myStave=stavedrawing.stave(self.origin,self.height,width=self.GetSize()[0],words=self.model.words,notes=self.model.notes,parent=self,fitInWindow=True,interactive=True,matchesNotes=self.model.matchesWord(self.model.notes))
        staveLength=myStave.getOverallLength(dc)
        if staveLength>0.9*self.GetSize()[0]:
            myStave.setOrigin((0.9*self.GetSize()[0]-staveLength,self.origin[1]))
        myStave.setColour(colours.staff,colours.staffLines)
        myStave.draw(dc)

#        if staveLength>0.9*self.GetSize()[0]:
#            width=self.GetSize()[0]*(float(self.GetSize()[0])/float(staveLength))
#            start=self.GetSize()[0]-width
#            scrollbar=drawutils.HScrollMeter(start,width,(self.GetSize()[0],10),(0,self.GetSize()[1]-12))
#            scrollbar.draw(dc)        

        fonts.resetFont(dc,pts,weight)

        self.__borderDrawing(dc)

    def commandUpdate(self):
        print "StaffPanel: update"
        dc=self.__getClientDC();
        dc.SetFont(self.font)
        dc.Clear()
        self.draw(dc)

    def setOrigin(self):
        calcpos=(self.GetSize()[1]-self.height)*.4
        ypos=max([20,calcpos])
        self.origin=(0,ypos)

    def statusUpdate(self,text):
        print 'StaffPanel: updateStatus',text
        self.agent.updateStatus(text)

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
 
class CommandPanel(wx.Window):
    def __init__(self,parent,size,agent,style=wx.BORDER_NONE):
        wx.Window.__init__(self,parent,-1,size=size,style=style|wx.WANTS_CHARS)
        self.SetCursor(wx.StockCursor(wx.CURSOR_IBEAM))
        self.SetBackgroundColour(colours.commandlineBackground)
        self.Bind(wx.EVT_PAINT,self.OnPaint)
        self.Bind(wx.EVT_LEFT_DOWN,self.onLeftDown)
        self.Bind(wx.EVT_LEFT_UP,self.onLeftUp)
        self.Bind(wx.EVT_MOTION,self.onMotion)
        self.Bind(wx.EVT_CHAR,self.OnChar)
 
        self.blink=False
        self.timer=wx.Timer(self,1001)
        self.Bind(wx.EVT_TIMER,self.onTimer,id=1001)
        self.timer.Start(500)
 
        self.agent=agent
        self.listener=agent.commandModel
        
        welcome=' eigencommander command line'
        self.maxLines=50
        self.lines=[TextLine(self), TextLine(self,welcome)]
        self.selectionStarted=False
        self.selectingLine=0

        #self.selecting=False
        self.displayManager=DisplayLineManager(self)

        self.popupmenu=wx.Menu()
        for text in "Cut Copy Paste".split():
            item=self.popupmenu.Append(-1,text)
            self.Bind(wx.EVT_MENU, self.OnPopupItemSelected,item)
        self.Bind(wx.EVT_CONTEXT_MENU,self.OnShowPopup)

        self.font=wx.Font(13,wx.FONTFAMILY_MODERN,wx.FONTSTYLE_NORMAL,weight=wx.FONTWEIGHT_NORMAL)

        self.history=[]
        self.historyIndex=0
        self.SetFocus()

    def __getClientDC(self):
        if self.IsDoubleBuffered():
           dc=wx.ClientDC(self)
        else:
           dc=wx.BufferedDC(wx.ClientDC(self))
        return dc

    def __backgroundDrawing(self,dc):
        size=self.GetClientSize()
        drawutils.setPenColour(dc,colours.borderGradient1)
        drawutils.setBrushColour(dc,colours.borderGradient1)
        dc.DrawRectangle(0,0,size[0],size[1])


    def OnShowPopup(self,evt):
        if self.isSelecting():
            if self.getSelecting()==0:
                self.popupmenu.Enable(self.popupmenu.FindItem('Cut'),True)
                self.popupmenu.Enable(self.popupmenu.FindItem('Copy'),True)
                self.popupmenu.Enable(self.popupmenu.FindItem('Paste'),True)
            else:
                self.popupmenu.Enable(self.popupmenu.FindItem('Cut'),False)
                self.popupmenu.Enable(self.popupmenu.FindItem('Copy'),True)
                self.popupmenu.Enable(self.popupmenu.FindItem('Paste'),False)
     
        else:
            self.popupmenu.Enable(self.popupmenu.FindItem('Cut'),False)
            self.popupmenu.Enable(self.popupmenu.FindItem('Copy'),False)
            self.popupmenu.Enable(self.popupmenu.FindItem('Paste'),True)

        pos=evt.GetPosition()
        pos=self.ScreenToClient(pos)
        self.PopupMenu(self.popupmenu,pos)

    def OnPopupItemSelected(self,evt):
        item=self.popupmenu.FindItemById(evt.GetId())
        text=item.GetText()
        if text=='Cut':
            self.doCut()
            self.drawText(self.__getClientDC())
        elif text=='Copy':
            self.doCopy()
            self.drawText(self.__getClientDC())
        elif text=='Paste':
            self.doPaste()
            self.drawText(self.__getClientDC())

    def updateStatus(self,text):
        if text=='Belcanto Interpreter connected':
            self.timer.Start(500)
        else:
            self.timer.Stop()
 
    def onLeftDown(self,evt):
        self.SetFocus()

        dc=wx.ClientDC(self)
        dc.SetFont(self.font)
        mPos=evt.GetPositionTuple()
        for line in self.lines:
            line.selecting=False

        r=self.displayManager.mPos2Index(mPos,dc) 
        if r:
            lineNo=r[0]
            index=r[1]
            print 'returned index=',index,'in line',lineNo
            # XXX restrict to active line initially
#            if lineNo==0:
#                self.lines[0].selecting=True
#                self.lines[0].mouseIndex=index
#                self.lines[0].selectionLeft=index
#                self.lines[0].selectionRight=index
#                self.drawText(self.__getClientDC())

#            for line in self.lines:
#                line.selecting=False

            self.selectionStarted=True
            self.selectingLine=lineNo
#            self.lines[lineNo].selecting=True
            self.lines[lineNo].mouseIndex=index
            self.lines[lineNo].selectionLeft=index
            self.lines[lineNo].selectionRight=index
            self.drawText(self.__getClientDC())


    def onLeftUp(self,evt):
        self.selectionStarted=False
        self.selectingLine=0
        mPos=evt.GetPositionTuple()

    def isSelecting(self):
        count=0
        for line in self.lines:
            if line.selecting:
                print count
                return True
            count=count+1
        return False

    def getSelecting(self):
        count=0
        for line in self.lines:
            if line.selecting:
                return count
            count=count+1
        return None
      
    def onMotion(self,evt):
        #if evt.Dragging() and evt.LeftIsDown() and self.isSelecting():
        if evt.Dragging() and evt.LeftIsDown() and self.selectionStarted:

            dc=wx.ClientDC(self)
            dc.SetFont(self.font)
            mPos=evt.GetPositionTuple()
            r=self.displayManager.mPos2Index(mPos,dc) 

            if r:
               index=r[1]
               lineNo=r[0]
#                print 'on Motion'
#                print 'returned index=',index,'in line',lineNo
#                # XXX restrict to active line initially
#                if lineNo==0:
#                    self.lines[0].doMouseMove(index)
#                    self.drawText(self.__getClientDC())

               if lineNo==self.selectingLine:
                   self.lines[lineNo].selecting=True
                   self.lines[lineNo].doMouseMove(index)
                   self.drawText(self.__getClientDC())

    def onTimer(self,evt):
        self.blink=not self.blink
#	print 'onTimer',self.blink
        if self.displayManager.cursorStarted:
#            dc=self.__getClientDC()
            dc=wx.ClientDC(self)
            self.displayManager.blinkCursor(dc)

    def OnChar(self,evt):
        #first check for special keys
        key= evt.GetKeyCode()
        if key==wx.WXK_RETURN:
            self.onEnter()
        elif key==wx.WXK_LEFT:
            if evt.ShiftDown():
                self.onLeftShift()
            else:
                self.onLeft()
        elif key==wx.WXK_RIGHT:
            if evt.ShiftDown():
                self.onRightShift()
            else:
                self.onRight()
        elif key==wx.WXK_UP:
            self.onPrev()
        elif key==wx.WXK_DOWN:
            self.onNext()
        elif key==wx.WXK_BACK:
            self.onBack()
        elif key==wx.WXK_ESCAPE:
            self.onEscape()
        
        # Then do the printing characters
        else:
            char=self.getKeyName(evt)
            #print 'char=',char
            if char:
                if evt.CmdDown():
                    if char.lower()=='c':
                        self.doCopy()
                    elif char.lower()=='x':
                        self.doCut()
                    elif char.lower()=='v':
                        self.doPaste()
                else:
                    #self.insertText(char)
                    self.lines[0].insertText(char)
           
        self.drawText(self.__getClientDC())

    def doCut(self):
        self.lines[0].cut()

    def doCopy(self):
        for line in self.lines:
            if line.selecting:
                line.copy()
#        self.lines[0].copy()

    def doPaste(self):
        self.lines[0].paste()

    def onEnter(self):
        self.execute()
        self.lines[0].selecting=False

    def execute(self):
        self.historyIndex=0
        if not self.lines[0].text==self.lines[0].prompt: 

            self.updateHistory(self.lines[0].text)
            com=self.lines[0].getCommandString()
            self.updateLine('')
            self.sendCommand(com)

        self.updateLine(self.lines[0].prompt)
        self.lines[0].resetInsertion()
        #self.lines[0].insertionPoint=len(self.lines[0].text)
    
    def onLeft(self):
       for line in self.lines:
            line.selecting=False
       self.lines[0].onLeft()

    def onLeftShift(self):
       for line in self.lines:
            line.selecting=False
       self.lines[0].onLeftShift()
       
    def onRight(self):
       for line in self.lines:
            line.selecting=False
       self.lines[0].onRight()

    def onRightShift(self):
       for line in self.lines:
            line.selecting=False
       self.lines[0].onRightShift()

    def onEscape(self):
        self.lines[0].onEscape()
        self.historyIndex=0
        self.drawText(self.__getClientDC())
        for line in self.lines:
            line.selecting=False

    def onPrev(self):
        if self.history:
            if self.historyIndex==0:
                self.pending=self.lines[0].text
            if self.historyIndex<len(self.history):
                self.historyIndex=self.historyIndex+1
            self.lines[0].text=self.history[len(self.history)-self.historyIndex]
            self.lines[0].resetInsertion()
#            self.lines[0].insertionPoint=len(self.lines[0].text)
            self.drawText(self.__getClientDC())
        for line in self.lines:
            line.selecting=False

    def onNext(self):
        if self.history:
            if self.historyIndex>=1:
                self.historyIndex=self.historyIndex-1
            if not self.historyIndex==0:
                self.lines[0].text=self.history[len(self.history)-self.historyIndex]
            else:
                self.lines[0].text=self.pending
        self.lines[0].resetInsertion()
        #self.lines[0].insertionPoint=len(self.lines[0].text)
        self.drawText(self.__getClientDC())
        for line in self.lines:
            line.selecting=False

    def onBack(self):
        self.lines[0].onBack()

    def sendCommand(self,str):
#        print 'commandline: sendcommand'
        if self.listener:
            self.listener.inject(str)

    def updateLine(self,str):
        if not self.lines[0].text=='':
            if len(self.lines)<self.maxLines:
                self.lines.append(TextLine(self))
            
            for i in range(len(self.lines)-1,0,-1):
                self.lines[i].text=self.lines[i-1].text
            
        self.lines[0].set_text(str)

    def updateHistory(self,str):
        self.history.append(str)

    def getMaxWidth(self):
        return 0.9*self.GetSize()[0]

    def drawText(self,dc):
        font=self.font
        dc.SetFont(font)
        dc.SetTextForeground(colours.commandlineText)
        dc.Clear()
       
        self.__backgroundDrawing(dc)
        self.displayManager.clear()
        self.displayManager.load(dc,self.lines)
        self.displayManager.draw(dc)
        self.displayManager.drawCursor(dc,self.lines[0].insertionPoint)

        self.__borderDrawing(dc)

    def OnPaint(self,evt):
        dc=wx.AutoBufferedPaintDC(self)
        dc.SetFont(self.font)
        self.drawText(dc)
        evt.Skip()

    def __borderDrawing(self,dc):
        size=self.GetClientSize()
        pen=wx.Pen(colours.border_shadow_dark)
        pen.SetWidth(2)
        pen.SetCap(wx.CAP_BUTT)
        dc.SetPen(pen)
        dc.DrawLine(1,0,1,size[1])
        #dc.DrawLine(1,size[1]-1,size[0],size[1]-1)
        pen=wx.Pen(colours.defaultPen)
        pen.SetWidth(1)
        dc.SetPen(pen)
    
    def getKeyName(self, evt):
        keycode = evt.GetKeyCode()
        keyname=None
        
        if "unicode" in wx.PlatformInfo:
             keycode = evt.GetUnicodeKey()
             if keycode <= 127:
                 keycode = evt.GetKeyCode()
             keyname = unichr(evt.GetUnicodeKey())
                
        elif keycode < 256:
                keyname = chr(keycode)
        return keyname


