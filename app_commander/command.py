
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
from pigui import language

class CommandModel(language.LanguageDisplayModel): 
    def __init__(self,langmodel):
        language.LanguageDisplayModel.__init__(self,langmodel)
        self.__listeners = []
        self.words=[]
        self.notes=[]

    def addCommandListener(self,listener):
        if listener not in self.__listeners:
            self.__listeners.append(listener)

    def inject(self,msg):
        gui.call_bg_async(self.langmodel.inject,msg)

    def cmdline_changed(self,cmdline):
        print 'CommandModel:cmdline_changed',cmdline
        words=cmdline[0].split()
        self.words=self.getWordTuples(words)
        notes=str(cmdline[1])
        self.notes=list(notes)
        self.updateCommand()

    def updateCommand(self):
        print 'CommandModel:update listener',self.words,self.notes,self.__listeners
        for listener in self.__listeners:
            listener.commandUpdate()

    def language_disconnected(self):
        print 'LanguageDisplayModel:language_disconnected'
        for listener in self.__listeners:
            listener.statusUpdate("No Belcanto Interpreter")
        
    def language_ready(self,name):
        print 'LanguageDisplayModel:language_ready'
        for listener in self.__listeners:
            print 'update status on  listeners'
            listener.statusUpdate( "Belcanto Interpreter connected")

    def language_gone(self,name):
        print 'LanguageDisplayModel:language_gone'
        for listener in self.__listeners:
            listener.statusUpdate( "Belcanto Interpreter disconnected")
