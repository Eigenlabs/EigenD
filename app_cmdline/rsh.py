
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

import sys,threading,piw
from pisession import session
from pi import proxy,async,logic,langproxy
from pibelcanto import lexicon

def reverse_lexicon():
    r={}
    for e,(m,t) in lexicon.lexicon.iteritems(): r[m]=e
    return r

class Browser(langproxy.LanguageProxy):
    def __init__(self,address):
        langproxy.LanguageProxy.__init__(self,address)
        self.__connected = False

    def language_ready(self):
        print 'language agent connected'
        self.__connected = True

    def language_gone(self):
        print 'language agent disconnected'
        self.__connected = False

    def run(self,func,*args,**kwds):
        d = func(*args,**kwds)
        e = threading.Event()

        def ok(*a,**k):
            e.set()

        def notok(*a,**k):
            e.set()

        d.setCallback(ok).setErrback(notok)
        return e

    @async.coroutine()
    def __cmd(self,line):
        r=(yield self.invoke_rpc('exec',line))

        if not r.status():
            print 'rpc error, exec',r.args()
            yield async.Coroutine.success()

        yield async.Coroutine.success()

    def __nullcmd(self):
        e = threading.Event()
        e.set()
        return e

    def process(self,line):
        if not self.__connected:
            print 'not connected'
            return self.__nullcmd()
            
        return self.run(self.__cmd,line)

class Cli(threading.Thread):

    def __init__(self,address):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self.__address=address
        self.__event = threading.Event()
        self.__reverse = reverse_lexicon()

    def tomusic(self,line):
        m=[]
        e=[]

        for w in line.split():
            ew = self.__reverse.get(w)
            if ew is None:
                print w,'not in lexicon'
                return (None,None)
            m.append('!'+w)
            e.append(ew)

        return (' '.join(m),' '.join(e))

    def process_line(self, line):
        line = line.strip()

        if not line:
            return

        if line.startswith('?'):
            self.translate(line[1:].strip())
            return

        (mline,eline) = self.tomusic(line)

        if not mline:
            return 'invalid phrase'

        print '  (',eline,')'

        piw.tsd_lock()

        try:
            event = self.browser.process(mline)
        finally:
            piw.tsd_unlock()

        event.wait()

    def translate_word(self,w):
        return lexicon.lexicon.get(w.lower(),('unknown','noun'))[0]

    def translate(self,l):
        mline = ' '.join([ self.translate_word(w) for w in l.split()])
        print '  (',mline,')'

    def startup(self,m):
        self.browser = Browser(self.__address)
        self.snapshot = piw.tsd_snapshot()
        self.__event.set()

    def run(self):
        session.run_session(self.startup,clock=False)

    def mainloop(self):

        self.start()
        self.__event.wait(5)

        if not self.__event.isSet():
            raise "thread didnt start"

        self.snapshot.install()

        while True:
            try:
                self.process_line(raw_input('> ').strip())
            except EOFError:
                break
            except KeyboardInterrupt:
                break

def main():
    if len(sys.argv) != 2:
        print 'usage: brcmd <language>'
        sys.exit(-1)

    Cli(sys.argv[1]).mainloop()
