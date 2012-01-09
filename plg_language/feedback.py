
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

from pi import atom,domain,node,utils
from pibelcanto import lexicon
from . import language_native
import piw

class History(node.Server):
    history = 10

    def __init__(self,callback):
        node.Server.__init__(self,rtransient=True)
        self.__buffer = []
        self.__current = ''
        self.__next = 1
        self.__callback = callback
        self.__recogniser = language_native.wordrec(utils.changify(self.__keyinput))

        for h in range(0,self.history):
            self[h+1] = node.Server()

        self.__setup()

    def __update(self,k,v):
        t = piw.tsd_time()
        v = piw.makestring(v,t)
        self.__callback.update_timestamp(t)
        if k is None:
            self.set_data(v)
        else:
            self[k].set_data(v)

    def clear_history(self):
        t = piw.tsd_time()
        v = piw.makestring('',t)
        self.__callback.update_timestamp(t)
        for h in range(0,self.history):
            self[h+1].set_data(v)

    def __keyinput(self,d):
        if not d.is_long():
            return

        note = d.as_long()
        if note >= 1000:
            note -= 1000
            hard = True
        else:
            hard = False

        if note < 9 and not hard:
            self.__current += chr(ord('0')+note)
            self.__setup()
            return
            
        if note < 9 and hard:
            self.__current += chr(ord('0')+note)
            self.__buffer.append(self.__current)
            w = "!%s" % self.__current
            self.__current = ''
            self.__setup()
            self.__callback.word_in(w)
            return

        if note == 9:
            self.__current = ''
            self.__buffer = []
            self.__setup()
            self.__callback.line_clear()
            return

        if note == 10 and self.__current:
            self.__current=self.__current[:-1]
            self.__setup()
            return

        if note == 10 and not self.__current:
            m = self.__callback.word_out()

            if not m:
                return

            self.__current = m[1:]

            if self.__buffer and self.__buffer[-1]==self.__current:
                self.__buffer = self.__buffer[0:-1]

            self.__current=self.__current[:-1]
            self.__setup()
            return

    def cookie(self):
        return self.__recogniser.cookie()

    def inject(self,word):
        self.__current = ''
        self.__buffer.append(word)
        w = "!%s" % word
        self.__setup()
        self.__callback.word_in(w)

    def __setup(self):
        line = ' '.join(['!%s'%w for w in self.__buffer])
        word = "!%s" % self.__current if self.__current else ""
        val = "%s:%s" % (line,word)
        self.__update(None,val)

    def __again(self,status,msg):
        n = self.__next-1
        s = 1+(n-1)%self.history
        v = self[s].get_data().as_string().split(':')
        v[1]+=' !4'
        v[2]='ok' if status else 'failed'
        v[3]=msg
        v=':'.join(v)
        self.__update(s,v)
        print 'again',s,v
        self.__buffer = []
        self.__setup()

    def __clearing(self,line):
        val=line=='!%s !%s' % (lexicon.lexicon['history'][0],lexicon.lexicon['clear'][0])
        print 'clearing history',val
        return val

    def message(self,msg,desc='message',speaker=''):
        print 'feedback:msg',msg
        n = self.__next
        s = 1+(n-1)%self.history
        self.__next+=1
        line = ' '.join(msg)
        val = '%d:%s:%s:%s:%s' % (n,line,desc,'',speaker)
        self.__update(s,val)

    def buffer_done(self,status,msg,repeat):
        if not self.__buffer:
            return

        if repeat:
            self.__again(status,msg)
            return

        print 'buffer_done',self.__buffer,status,msg
        n = self.__next
        s = 1+(n-1)%self.history
        self.__next+=1
        st = "ok" if status else "failed"
        line = ' '.join(['!%s'%w for w in self.__buffer])
        if not self.__clearing(line):
            val = '%d:%s:%s:%s:%s' % (n,line,st,msg,'')
            self.__update(s,val)

        self.__buffer = []
        self.__setup()
