
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

import piw
from pi import langproxy,index
import picross
from pigui import utils
from pisession import gui

class LanguageDisplayModel:
    def __init__(self,langmodel,listener=None):
        self.langmodel=langmodel
        self.listeners=[]
        if listener:
            self.addListener(listener)

        self.__addDisplay()

    def __addDisplay(self):
        gui.call_bg_async(self.langmodel.addDisplay,self)

    def close(self):
        self.langmodel.removeDisplay(self)
 
    def language_disconnected(self):
        print 'LanguageDisplayModel:language_disconnected'
        for listener in self.listeners:
            listener.updateStatus("No Belcanto Interpreter")
        
    def language_ready(self,name):
        print 'LanguageDisplayModel:language_ready'
        for listener in self.listeners:
            print 'update status on  listeners'
            listener.updateStatus( "Belcanto Interpreter connected")

    def language_gone(self,name):
        print 'LanguageDisplayModel:language_gone'
        for listener in self.listeners:
            listener.updateStatus( "Belcanto Interpreter disconnected")
 
    def addListener(self,listener):
        if not listener in self.listeners:
            self.listeners.append(listener)

    def removeListener(self,listener):
        if listener in self.listeners:
            self.listeners.remove(listener)

    def cmdline_changed(self,cmdline):
        pass
    
    def context_changed(self,name,listen,lurk):
        pass

    def history_changed(self,max):
        pass

    def get_history(self,k):
        pass

    def history_cleared(self):
        pass


class IndexModel(piw.index):

    def __init__(self,database=None):
        self.listeners=[]
        self.agentList=[]
        self.added=[]
        self.removed=[]
        self.database=database
        piw.index.__init__(self)

    def index_opened(self):
        if self.updateList():
            for listener in self.listeners:
                listener.indexUpdate(self.added,self.removed)

    def index_closed(self):
        print 'IndexModel','index_closed'
        
    def index_changed(self):
        if self.updateList():
            print 'Index model changed: updating listeners'
            for listener in self.listeners:
                listener.indexUpdate(self.added,self.removed)
    
    def updateList(self):
        listModified=False
        self.added=[]
        self.removed=[]
        newlist=[]
        for i in range(0,self.member_count()):
            name = self.member_name(i).repr()
            newlist.append(utils.stripQuotes(name))
        for name in newlist:
            if name not in self.agentList:
                print 'added',name
                self.agentList.append(name)
                self.added.append(name)
                listModified=True
        for name in self.agentList:
            if name not in newlist:
                print 'removed',name
                self.agentList.remove(name)
                self.removed.append(name)
                listModified=True
        return listModified

    def addListener(self,listener):
        self.listeners.append(listener)

    def removeListener(self,listener):
        if listener in self.listeners:
            self.listeners.remove(listener)

    def getName(self,id):
        if self.database:
            return self.database.find_desc(id)
   
    def getList(self):
        return self.agentList

    def root_changed(self,*args):
        pass


class LanguageModel:
    def __init__(self,connector=None):
        self.lang=None
        self.initialising=True
        self.agentIndex=IndexModel()
        piw.tsd_index('<language>',self.agentIndex)
        self.agentIndex.addListener(self)
        self.languageAgents=[]
        self.langName=''
        self.displays=[]
        self.status=0
        self.__connector = connector
 
    def update(self):
        self.languageAgents=self.agentIndex.getList()
        if self.languageAgents and self.initialising:
            self.connect(self.languageAgents[0])
        self.initialising =False

    def indexUpdate(self,added,removed):
        self.update()
    
    def connect(self,name):
        if self.lang:
            self.language_disconnected()
        self.lang=langproxy.LanguageProxy(name,delegate=self)
        self.langName=name

    def disconnect(self):
        self.lang.close_client()

    def history_changed(self,max):
        picross.display_active()
        #print 'history changed, max=',max
        for display in self.displays:
            gui.call_fg_async(display.history_changed,max)
    
    def history_cleared(self):
        picross.display_active()
        print 'history_cleared'
        for display in self.displays:
            gui.call_fg_async(display.history_cleared)
    
    def cmdline_changed(self):
        picross.display_active()
        cmdline=self.lang.get_cmdline()
        print 'LanguageModel:cmdline_changed, ',cmdline
        for display in self.displays:
            gui.call_fg_async(display.cmdline_changed,cmdline)

    def context_changed(self,name,listeners,lurkers):
        print 'context changed',name,listeners,lurkers
        for display in self.displays:
            gui.call_fg_async(display.context_changed,name,listeners,lurkers)

    def get_history(self,k):
        if self.lang:
            return [k,self.lang.get_history(k)]

    def language_ready(self):
        print 'LanguageModel:language_ready' 
        for display in self.displays:
            gui.call_fg_async(display.language_ready,self.langName)
        self.status=1
        if self.__connector is not None:

            h1=self.lang.roll_id(1)
            h2=self.lang.roll_id(2)
            
            v1=self.lang.yaw_id(1)
            v2=self.lang.yaw_id(2)

            a1=self.lang.activation_id(1)
            a2=self.lang.activation_id(2)
            a3=self.lang.activation_id(3)
            a4=self.lang.activation_id(4)
            a5=self.lang.activation_id(5)
            a6=self.lang.activation_id(6)

            h6=self.lang.roll_id(6)
            v6=self.lang.yaw_id(6)
            
            a1=self.lang.activation_id(1)
            gui.call_fg_async(self.__connector,h1,v1,h2,v2,a3,a4,a5,h6,v6,a1,a2,a6)
        self.lang.flush()

    def language_gone(self):
        print 'LanguageModel:language_gone' 
        for display in self.displays:
            gui.call_fg_async(display.language_gone,self.langName)
        self.status=0

    def language_disconnected(self):
        print 'LanguageModel:language_disconnected' 
        for display in self.displays:
            gui.call_fg_async(display.language_disconnected)
        self.status=2

    def addDisplay(self,display):
        if not display in self.displays:
            self.displays.append(display)
            if self.status==1:
                gui.call_fg_async(display.language_ready,self.langName)

    def removeDisplay(self,display):
        self.displays.remove(display)

    def inject(self,msg):
        print 'LanguageModel: inject',msg
        self.lang.inject(msg)

