
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
from pi import proxy,async,logic,action,const,paths

class Browser(proxy.AtomProxy):

    monitor = set(['timestamp'])

    def __init__(self,address):
        proxy.AtomProxy.__init__(self)
        self.__address = address
        self.__anchor = piw.canchor()
        self.__anchor.set_client(self)
        self.__anchor.set_address_str(address)
        self.__event = threading.Event()
        self.__event.clear()
        self.__errmsg=None
        self.__dir=[]
        self.__nf=0
        self.__nc=0

    def dir(self):
        if not self.__dir:
            return '/'
        return '/'.join(self.__dir)
        
    def node_ready(self):
        if 'browse' not in self.protocols():
            self.__errmsg = "%s not browseable" % self.__address

        self.__event.set()

    def node_changed(self,parts):
        if 'timestamp' in parts:
            print 'data changed'

    def wait(self):
        self.__event.wait(5)

        if not self.__event.isSet():
            raise "can't connect to",self.__address

        if self.__errmsg:
            raise self.__errmsg

        print 'connected to',self.__address

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
    def __lc(self):
        current = 0

        while current < self.__nc:
            a=logic.render_term((tuple(self.__dir),current))
            r=(yield self.invoke_rpc('cinfo',a))

            if not r.status():
                print 'rpc error, cinfo',r.args()
                yield async.Coroutine.success()

            clist = logic.parse_clause(r.args()[0])
            for c in clist:
                print c

            current = current+len(clist)

        yield async.Coroutine.success()

    @async.coroutine()
    def __st(self,start):
        a=logic.render_term((tuple(self.__dir),start))
        r=(yield self.invoke_rpc('finfo',a))

        if not r.status():
            print 'rpc error, finfo',r.args()
            yield async.Coroutine.success()

        try:
            flist = logic.parse_clause(r.args()[0])
        except:
            print 'cant parse:',r.args()[0]
            yield async.Coroutine.success()

        (cookie,desc,name)=flist[0]

        a=logic.render_term((tuple(__dir),cookie))
        r=(yield self.invoke_rpc('fideal',a))

        if not r.status():
            print 'rpc error, fideal',r.args()
            yield async.Coroutine.success()

        s=paths.make_subst(self.__address)
        print 's=',s

        try:
            ideal = logic.parse_clause(r.args()[0],s)
        except:
            print 'cant parse:',r.args()[0]
            raise
            yield async.Coroutine.success()

        print cookie,desc,name,ideal
        yield async.Coroutine.success()

    @async.coroutine()
    def __lf(self,start):
        current = start
        finish = start+10
        if finish > self.__nf: finish=self.__nf

        while current < finish:
            a=logic.render_term((tuple(self.__dir),current))
            r=(yield self.invoke_rpc('finfo',a))

            if not r.status():
                print 'rpc error, finfo',r.args()
                yield async.Coroutine.success()

            try:
                flist = logic.parse_clause(r.args()[0])
            except:
                print 'cant parse:',r.args()[0]
                yield async.Coroutine.success()

            for i,f in enumerate(flist):
                if i+current >= finish:
                    break
                print i+current,f

            current = current+len(flist)

        yield async.Coroutine.success()

    @async.coroutine()
    def __cd(self,path):
        a=logic.render_term(tuple(path))
        r=(yield self.invoke_rpc('enumerate',a))

        if not r.status():
            print 'rpc error, enumerate',r.args()
            yield async.Coroutine.success()

        (nf,nc) = logic.parse_clause(r.args()[0])

        (self.__dir,self.__nf,self.__nc) = (path,nf,nc)

        print self.dir(),'files=',self.__nf,'collection=',self.__nc
        yield async.Coroutine.success()

    def __nullcmd(self):
        e = threading.Event()
        e.set()
        return e

    def __help(self):
        print 'cd - change to root'
        print 'cd <cmp> - change to <cmp>'
        print 'st <num> - stat collections'
        print 'lc - list collections'
        print 'lf - list 10 collections starting from 0'
        print 'lf <num> - list 10 collections starting from <num>'
        return self.__nullcmd()

    def process(self,line):
        if line is None:
            self.__dir=[]
            self.__help()
            return self.run(self.__cd,[])

        w = line.strip().split()

        if len(w)==1 and w[0]=='lc':
            return self.run(self.__lc)

        if len(w)==2 and w[0]=='st':
            return self.run(self.__st,int(w[1]))

        if len(w)==1 and w[0]=='lf':
            return self.run(self.__lf,0)

        if len(w)==2 and w[0]=='lf':
            return self.run(self.__lf,int(w[1]))

        if len(w)==1 and w[0]=='cd':
            return self.run(self.__cd,[])

        if len(w)==1 and w[0]=='help':
            return self.__help()

        if len(w)==2 and w[0]=='cd':
            p=self.__dir+[w[1]]
            return self.run(self.__cd,p)

        print line,'unrecognised'
        return self.__nullcmd()

class Cli(threading.Thread):

    def __init__(self,address):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self.__address=address
        self.__event = threading.Event()

    def process_locked(self, line):
        piw.tsd_lock()

        try:
            event = self.browser.process(line)
        finally:
            piw.tsd_unlock()

        event.wait()

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

        self.browser.wait()
        self.snapshot.install()

        self.process_locked(None)

        while True:
            try:
                self.process_locked(raw_input('%s> ' % self.browser.dir()).strip())
            except EOFError:
                break
            except KeyboardInterrupt:
                break

def main():
    if len(sys.argv) != 2:
        print 'usage: brsh <ideal>'
        sys.exit(-1)

    Cli(sys.argv[1]).mainloop()
