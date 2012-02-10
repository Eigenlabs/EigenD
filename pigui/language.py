
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
    def __init__(self,langmodel):
        self.langmodel=langmodel
        self.__addDisplay()

    def matchesWord(self,notes):
        music=''
        for note in notes:
            if note!='!':
                music=music+note
 
        reverse = self.get_reverse_lexicon()
        matches={}

        for key in reverse:
            if key[:len(music)]==music and len(key)>len(music):
                matches[key]=reverse[key][0]

        return len(matches)>0        

    def getWordTuples(self,words):
        tuples=[]
        reverse = self.get_reverse_lexicon()

        for word in words:
            english='***'
            if word[1:] in reverse:
                english=reverse[word[1:]][0]
            tuples.append((word,english))

        return tuples
 
   
    def getMusic(self,english):
        forward = self.get_lexicon()

        if english in forward:
            return '!'+forward[english][0]
        else:
            return ''

    def getEnglish(self,music):
        reverse = self.get_reverse_lexicon()

        if music in reverse:
            return reverse[music][0]
        else:
            return ''

    def words_to_notes(self,str):
        words=[]
        w=[]
        w=str.split()
        
        for m in w:
            if m[0].isdigit() or m[0]=='-':
                for n in list(m):
                    words.append((self.getMusic(n),n)) 
            else:
                words.append((self.getMusic(m),m))
        
        return words

    def __addDisplay(self):
        gui.call_bg_async(self.langmodel.addDisplay,self)

    def close(self):
        self.langmodel.removeDisplay(self)
 
    def get_reverse_lexicon(self):
        return self.langmodel.get_reverse_lexicon()

    def get_lexicon(self):
        return self.langmodel.get_lexicon()

    def language_disconnected(self):
        pass
        
    def language_ready(self,name):
        pass

    def language_gone(self,name):
        pass
 
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

    def lexicon_changed(self):
        print 'default lexicon changed'


class IndexModel(piw.index):

    def __init__(self,database=None):
        self.__listeners=[]
        self.agentList=[]
        self.added=[]
        self.removed=[]
        self.database=database
        piw.index.__init__(self)

    def index_opened(self):
        if self.updateList():
            for listener in self.__listeners:
                listener.indexUpdate(self.added,self.removed)

    def index_closed(self):
        print 'IndexModel','index_closed'
        
    def index_changed(self):
        if self.updateList():
            print 'Index model changed: updating listeners'
            for listener in self.__listeners:
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

    def addIndexListener(self,listener):
        self.__listeners.append(listener)

    def removeIndexListener(self,listener):
        if listener in self.__listeners:
            self.__listeners.remove(listener)

    def getName(self,id):
        if self.database:
            return self.database.find_desc(id)
   
    def getList(self):
        return self.agentList

    def root_changed(self,*args):
        pass


class LanguageModel:
    def __init__(self):
        self.lang=None
        self.initialising=True
        self.agentIndex=IndexModel()
        piw.tsd_index('<language>',self.agentIndex)
        self.agentIndex.addIndexListener(self)
        self.languageAgents=[]
        self.langName=''
        self.displays=[]
        self.status=0
 
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

    def get_reverse_lexicon(self):
        if self.lang:
            return self.lang.get_reverse_lexicon()
        return {}

    def get_lexicon(self):
        if self.lang:
            return self.lang.get_lexicon()
        return {}

    def disconnect(self):
        self.lang.close_client()

    def history_changed(self,max):
        picross.display_active()
        #print 'history changed, max=',max
        for display in self.displays:
            gui.call_fg_async(display.history_changed,max)
    
    def lexicon_changed(self):
        picross.display_active()
        print 'lexicon_changed'
        #for display in self.displays:
        #    gui.call_fg_async(display.lexicon_changed)
    
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
        self.lang.flush()

    def lexicon_changed(self):
        print 'LanguageModel:lexicon_changed'
        for display in self.displays:
            gui.call_fg_async(display.lexicon_changed)

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

