
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
from pigui import colours,fonts,utils,drawutils,stavedrawing,panels,language
from pi import atom,action,async,errors

def getRoles():
    return ()

def getName():
    return 'dictionary'

def getView(factory,args,parent):
    return DictionaryPanel(parent,factory)

class DictionaryModel(language.LanguageDisplayModel):
    def __init__(self,langModel):
        language.LanguageDisplayModel.__init__(self,langModel)
        self.byWord=True
        self.m2e={}
        self.e2m={}
        self.e2mkeys = []
        self.m2ekeys = []
        self.__listeners = []

    def addDictionaryListener(self,listener):
        if listener not in self.__listeners:
            self.__listeners.append(listener)

    def update(self):
        for listener in self.__listeners:
            listener.dictionaryUpdate()
            
    def lexicon_changed(self):
        print 'dictionary model lexicon changed'
        language.LanguageDisplayModel.lexicon_changed(self)
        self.set_lexicon(self.get_lexicon())
        self.update()

    def set_lexicon(self,lex):
        for (e,(m,t)) in lex.iteritems():
            if m and e:
                self.m2e[m]=e
                self.e2m[e]=m

        print 'Dictionary length', len(self.m2e)
        self.e2mkeys=self.e2m.keys()
        self.e2mkeys.sort()
        self.m2ekeys=self.m2e.keys()
        self.m2ekeys.sort()

    def getDict(self):
        if self.byWord:
            return self.e2m
        else:
            return self.m2e

    def setByWord(self,byWord):
        self.byWord=byWord

    def getKeys(self):
        if self.byWord:
            return self.e2mkeys
        else:
            return self.m2ekeys

class DictionaryAgent(atom.Null):
    def __init__(self,parent,name):
        atom.Null.__init__(self,names=name,container=(None,'dict',parent.verb_container()))
        self.model=parent.dictionaryModel
        self.add_verb2(1,'show([],None,role(by,[abstract,matches([word])]))',callback=self.sortByWord)
        self.add_verb2(2,'show([],None,role(by,[abstract,matches([note])]))',callback=self.sortByNote)
 
    def set_lexicon(self,lex):
        self.model.set_lexicon(lex)
        self.model.update()

    def sortByWord(self,*args):
        if self.model.byWord:
            return async.success(errors.nothing_to_do('show'))
        else:
            self.model.setByWord(True)
            self.model.update()

    def sortByNote(self,*args):
        if self.model.byWord:
            self.model.setByWord(False)
            self.model.update()
        else:
            return async.success(errors.nothing_to_do('show'))
            
class DictionaryPanel(wx.Window):
    def __init__(self,parent,size,agent,style=wx.BORDER_NONE | wx.HSCROLL):
        wx.Window.__init__(self,parent,-1,size=size,style=style)  
#self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)
        self.SetBackgroundColour(colours.borderGradient1)
        self.Bind(wx.EVT_PAINT, self.onPaint)
        self.Bind(wx.EVT_IDLE,self.onIdle)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_SCROLLWIN, self.onScroll)
        self.Bind(wx.EVT_MOUSEWHEEL,self.onMouseWheel)
#        self.Bind(wx.EVT_SET_FOCUS,self.OnSetFocus)

        self.agent=agent
        self.xMargin=10
        self.yMargin=10
        self.yScroll=1
        self.yScrollCentre=1
        self.scrollRange=0
        self.drawingItems=[]
        self.oldXOffset=0
        self.__updateRequired=False
        self.staveManager=stavedrawing.staveDrawingManager()

        self.xScroll=-1
        self.minX=0
        self.maxX=0
        self.minY=0
        self.maxY=0

        self.model=agent.dictionaryModel
        self.model.addDictionaryListener(self)
        self.SetScrollbar(wx.HORIZONTAL,0,10,9)

#    def OnSetFocus(self,evt):
#        print 'dictionary OnSetFocus'
#        self.GetParent().doFocus()
#        evt.Skip()

    def onIdle(self,evt):
        if self.__updateRequired:
            self.dictionaryUpdate()

    def __getClientDC(self):
        if self.IsDoubleBuffered():
           dc=wx.ClientDC(self)
        else:
           dc=wx.BufferedDC(wx.ClientDC(self))
        return dc

    def onScroll(self,evt):
        print 'onScroll: evtPos=',evt.GetPosition(),'scrollRange=',self.scrollRange
        offset=2*(float(evt.GetPosition())/float(self.maxX))-1

        self.scrollbarScroll(offset)

    def onMouseWheel(self,evt):
        delta=evt.GetWheelDelta()
        rotation=evt.GetWheelRotation()
        print 'onMouseWheel:delta=',delta,'rotation=',rotation
#        deltaOffset=rotation*delta*evt.GetLinesPerAction()
#        self.wheelScroll(deltaOffset)

#    def wheelScroll(self,deltaOffset):
#        offset=self.offset-deltaOffset
#        numFiles=self.model.numFiles+self.model.numCollections
#        if offset>=0 and offset<=((numFiles+1)-self.__numItemsDisplayed):
#            self.offset=offset
#            self.fl.setOffset(self.offset)
#            self.__drawRequired=True

    def scrollbarScroll(self,offset):
        print 'scrollbarScroll',offset
        self.xScroll=offset
#        self.agent.reset1(h=offset)
#        dc=wx.ClientDC(self)
        dc=self.__getClientDC()
        dc.Clear()
        dc.SetFont(self.agent.font)
        self.doScrollDrawing(dc)
        self.__borderDrawing(dc)

    def OnSize(self,evt):
        print 'dictionaryPanel: OnSize'
        self.__updateRequired=True
        evt.Skip()

    def scroll_both(self,h,v):
        print 'DictionaryPanel scroll_both'
#        panels.ViewDrawing.scroll_both(self,h,None)

#    def close(self):
#        self.model.removeListener(self)

#    def open(self):
#        self.model.addListener(self)

    def dictionaryUpdate(self):
        print 'DictionaryPanel:update'
        self.__updateRequired=False
        dc=self.__getClientDC()
        self.drawingItems=[]
        self.oldXOffset=0
        self.staveManager.clear()
        dc.Clear()
        self.doPaint(dc)
        self.Refresh()

    def onPaint(self,evt):
        dc=self.__getClientDC()
        dc.Clear()
        self.doPaint(dc)
        evt.Skip()

    def doPaint(self,dc):
        dc.SetFont(self.agent.font)
        self.doScrollDrawing(dc)
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
 
    def updateStatus(self,str):
        pass
 
    def getName(self):
        return getName()

#    def getHelp(self):
#        return ['by word show', 'by note show']

    def getLineHeight(self,dc):
        dc.SetFont(self.agent.font)
        return int(2.6*dc.GetTextExtent("0")[1])

    def swapDict(self,dict):
        newDict={}
        for (k,v) in dict.iteritems():
            newDict[v]=k
        return newDict
    
    
#    def getMaxX(self,dc):
#        return dc.MaxX()+50-self.GetClientSize()[0]

#    def preDraw(self):
#        self.drawRequired=True

    def doDrawing(self,dc):
        self.__backgroundDrawing(dc)
        print 'dictionary: doDrawing'
        self.drawingItems=[]
        self.oldXOffset=0
        self.staveManager.clear()
        size=self.GetClientSize()
        if self.model:
            self.xMargin=3
            self.yMargin=10

            xpos=self.xMargin
            ypos=self.yMargin
            
            dict=self.model.getDict()
            numItems=len(dict)

            space=dc.GetTextExtent('00')[0]
            stavelen=space*3

            keys=self.model.getKeys()

            if keys:
                labeling=False
                label=''
                cols=1
                largestWidth=0
                widthSum=0

                for i in range(len(keys)):
                    key =keys[i]
                    if self.model.byWord:
                        music=dict[key]

                        if key[0]=='a':
                            labeling=True
                        if labeling and label != key[0]:
                            label=key[0]
                            self.drawingItems.append(drawutils.dictionaryLabel(label,xpos,ypos))
                            inc=self.getLineHeight(dc)
                            ypos=ypos+inc
                            lastInc=0.7*self.getLineHeight(dc)

                            if ypos+lastInc>self.GetClientSize()[1]:
                                ypos=self.yMargin
                                cols=cols+1
                                widthSum=widthSum+largestWidth

                                xpos=(self.xMargin)*cols+widthSum
                                largestWidth=0

                                self.drawingItems.append(drawutils.divider(xpos,0,xpos,size[1]))
                    
                    else:
                        music=key
                    
                    origin=(xpos,ypos)
                    staveHeight=1.6*dc.GetTextExtent('0')[1]

                    stave=stavedrawing.stave(origin,staveHeight,width=stavelen,words=[(music,'')],margins=(0.1,0.1),translation=False,numbers=True,matchesNotes=True)
                    stave.setColour(colours.staff,colours.staffLines)
                    self.staveManager.addStave(stave)

                    textPos=xpos+stavelen+(0.3*space)
                    if self.model.byWord:
                        self.drawingItems.append(drawutils.textItem(key,textPos,ypos))
                        width=stavelen+(0.3*space)+dc.GetTextExtent(key)[0]
                    else:
                        self.drawingItems.append(drawutils.textItem(dict[key],textPos,ypos))
                        width=stavelen+(0.3*space)+dc.GetTextExtent(dict[key])[0]

                    if width>largestWidth:
                        largestWidth=width
         
                    inc=self.getLineHeight(dc)
                    ypos=ypos+inc
                    lastInc=0.7*self.getLineHeight(dc)

                    if ypos+lastInc>self.GetClientSize()[1]:
                        ypos=self.yMargin
                        cols=cols+1
                        widthSum=widthSum+largestWidth

                        xpos=(self.xMargin)*cols+widthSum
                        largestWidth=0

                        self.drawingItems.append(drawutils.divider(xpos,0,xpos,size[1]))

                cols=cols+1
                widthSum=widthSum+largestWidth
                xpos=((self.xMargin)*cols+widthSum)

                self.drawingItems.append(drawutils.divider(xpos,0,xpos,size[1]))
        self.__drawImpl(dc)
        self.minX=0
        self.maxX=xpos+50-self.GetClientSize()[0]

#    def getMaxX(self,dc):
#        return dc.MaxX()+50-self.GetClientSize()[0]

    def __backgroundDrawing(self,dc):
        size=self.GetClientSize()
        drawutils.setPenColour(dc,colours.borderGradient1)
        drawutils.setBrushColour(dc,colours.borderGradient1)
        dc.DrawRectangle(0,0,size[0],size[1])

    def test_scroll_both(self,h,v,mouse=False):
        print 'dictionaryPanel:test_scroll_both',h,v
        self.xScroll=h
        self.yScroll=v
        # XXX testing 
        dc=self.__getClientDC()
        #dc=wx.ClientDC(self)
        dc.Clear()
        dc.SetFont(self.agent.font)
        self.doScrollDrawing(dc)
        self.__borderDrawing(dc)


    def getXScrollPos(self,minLimit=True):
        d=self.xScroll
        xScrollPos=(d+1)*0.5*(self.maxX-self.minX)+self.minX
        raw=xScrollPos

        if minLimit:
            if xScrollPos<0:
                xScrollPos=0
               
        xScrollPos=int(xScrollPos)
        #print 'getXScrollPos: xScrollPos',xScrollPos,'maxX',self.maxX,'minX',self.minX
        return xScrollPos

    def doScrollDrawing(self,dc):
        print 'Dictionary: doScrollDrawing'
        if self.drawingItems:
	    self.__backgroundDrawing(dc)
            xOffset=self.getXScrollPos()
            dxOffset=xOffset-self.oldXOffset
            self.oldXOffset=xOffset

            for item in self.drawingItems:
                item.setXOffset(dxOffset)
                item.draw(dc)

            self.staveManager.scrolldraw(dc,xOffset=dxOffset,xRange=(-50,self.GetClientSize()[0]+50))
        else:
            xOffset=self.getXScrollPos()
            self.doDrawing(dc)
        scrollbarpos=xOffset
        scrollbarthumb=self.GetClientSize()[0]
        self.scrollRange=self.maxX+self.GetClientSize()[0]-50
        print 'setScrollBar:',scrollbarpos,scrollbarthumb,self.scrollRange
        self.SetScrollbar(wx.HORIZONTAL,scrollbarpos,scrollbarthumb,self.scrollRange)

    def __drawImpl(self,dc):
        for item in self.drawingItems:
            item.draw(dc)

        self.staveManager.draw(dc)

    def getTitle(self):
        return 'Dictionary'
