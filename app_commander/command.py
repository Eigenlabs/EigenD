
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

class CommandModel(language.LanguageDisplayModel): 
    def __init__(self,langmodel,listener=None):
        language.LanguageDisplayModel.__init__(self,langmodel,listener)
        self.words=[]
        self.notes=[]
        self.vocab=vocab.Vocabulary()

    def inject(self,msg):
        gui.call_bg_async(self.langmodel.inject,msg)

    def cmdline_changed(self,cmdline):
        print 'CommandModel:cmdline_changed',cmdline
        words=cmdline[0].split()
        self.words=self.vocab.getWordTuples(words)
        notes=str(cmdline[1])
        self.notes=list(notes)
        self.update()

    def update(self):
        print 'CommandModel:update listener',self.words,self.notes,self.listeners
        for listener in self.listeners:
            listener.update()

