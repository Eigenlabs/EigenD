
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

from pisession import gui
from pigui import vocab, language

class HistoryModel(language.LanguageDisplayModel): 
    def __init__(self,langmodel,listener=None):
        language.LanguageDisplayModel.__init__(self,langmodel,None)
        self.maxItems=50
        self.vocab=vocab.Vocabulary()
        self.words=[]
        self.listeners=[]

    def history_cleared(self):
        self.clear()

    def clear(self):
        self.items=[]
        self.__lasti=0
        self.__maxRequested=0
        self.__lastwords=[]
        self.__laststatus=''
        self.update()

    def history_changed(self,max):
        self.get_history(max)

    def get_history(self,k):
        r=gui.defer_bg(self.langmodel.get_history,k)
        def history_ok(result):
            i=result[0]
            h=result[1]
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
                    self.words=words
                    for listener in self.listeners:
                        listener.update()
        
        def history_failed():
            print 'history failed'
        r.setCallback(history_ok).setErrback(history_failed)
    
    def __makeWords(self,w):
        if w.split()[0]=='*':
            return []
        words=[]

        for word in w.split():
            english='***'
            if word.startswith('!'):
                english=self.vocab.getEnglish(word[1:])
            else:
                english=word

            words.append((word,english))
        return words

    def get_text(self):
        line=''
        for word in self.words:
            eng=word[1]
            if eng=='***':
                eng=word[0]
                if eng.startswith('!'):
                    eng=eng[1:]
            line=line+eng+' '
        return line


