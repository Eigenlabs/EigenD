
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

from pi import async,const,logic,paths,files,rpc,proxy
from pi.logic.shortcuts import *
from pisession import gui
import piw

class BrowseProxy(proxy.AtomProxy):
    monitor = set(['timestamp'])

    def __init__(self,address,name,listener):
        print 'creating BrowseProxy for',address
        proxy.AtomProxy.__init__(self)
        self.__address=address
        self.__anchor=piw.canchor()
        self.__anchor.set_slow_client(self)
        self.__anchor.set_address_str(address)
        
        self.listeners=[listener]
        self.fileCache = files.FileCache(name)

    def shutdown(self):
        self.__anchor.set_address_str('')

    def node_ready(self):
        proxy.AtomProxy.node_ready(self)
        if 'browse' not in self.protocols():
            print "%s not browseable" % self.__address
        print 'node_ready',self.__address
        if self.listeners:
            for listener in self.listeners:
                gui.call_fg_async(listener.ready,self.__address)

    def node_changed(self,parts):
        proxy.AtomProxy.node_changed(self,parts)
        if 'timestamp' in parts:
            print 'node_changed',parts
            if self.listeners:
                for listener in self.listeners:
                    print 'BrowseProxy data changed',self.__address
                    gui.call_fg_async(listener.changed,self.__address)

    @async.coroutine('internal error')
    def getName(self,id):
        r=rpc.invoke_rpc(id,'displayname','')
        yield r

        if not r.status():
            print 'browse_db getName:not r.status'
            n=''
            if self.names():
                n=self.names()[0]
        else:
            n=r.args()[0]

        yield async.Coroutine.success(n)

    def addListener(self,listener):
        self.listeners.append(listener)

    def removeListener(self,listener):
        if listener in self.listeners:
            self.listeners.remove(listener)

    def removeAllListeners(self):
        self.listeners=[]

    @async.coroutine('internal error')
    def enumerate(self,id,path):
        a=logic.render_term(tuple(path))
        r=rpc.invoke_rpc(id,'enumerate',a)
        yield r

        if not r.status():
            yield async.Coroutine.failure('rpc error')

        nf,nc=logic.parse_clause(r.args()[0])
        yield async.Coroutine.success(nf,nc)

    @async.coroutine('internal error')
    def current(self,id):
        r=rpc.invoke_rpc(id,'current','')
        yield r

        if not r.status():
            yield async.Coroutine.failure('rpc error')

        c=logic.parse_clause(r.args()[0],paths.make_subst(id))
        yield async.Coroutine.success(c)
    
    @async.coroutine('internal error')
    def cinfo(self,id,path,start,ncolls):
        a=logic.render_term(tuple(path))
        r=rpc.invoke_rpc(id,'enumerate',a)
        yield r

        if not r.status():
            yield async.Coroutine.failure('rpc error')

        nf,nc=logic.parse_clause(r.args()[0])
        finish=start+ncolls
        if finish>nc:finish=nc

        colls=[]
        while start<finish:
            a=logic.render_term((tuple(path),start))
            r=rpc.invoke_rpc(id,'cinfo',a)
            yield r

            if not r.status():
                yield async.Coroutine.failure('rpc error')
       
            clist=logic.parse_clause(r.args()[0])

            if not clist:
                break

            colls.extend(clist)
            start=start+len(clist)

        colls=colls[:ncolls]
        yield async.Coroutine.success(colls,nc)


    @async.coroutine('internal error')
    def finfo(self,id,path):
        a=logic.render_term(tuple(path))
        r=rpc.invoke_rpc(id,'enumerate',a)
        yield r

        if not r.status():
            yield async.Coroutine.failure('rpc error')

        nf,nc=logic.parse_clause(r.args()[0])
        start=0
        finish=nf
        current=start

        files=[]
        while current < finish:
            a=logic.render_term((tuple(path),current))
            r=(yield rpc.invoke_rpc(id,'finfo',a))

            if not r.status():
                print 'rpc error, finfo',r.args()
                yield async.Coroutine.failure('rpc error')

            flist = logic.parse_clause(r.args()[0])

            if not flist:
                break

            files.extend(flist)
            current=current+len(flist)

        files=files[:nf]
        yield async.Coroutine.success(files,nf)

    @async.coroutine('internal error')
    def dinfo(self,id,path=[]):
        args=logic.render_term((tuple(path)))
        r=rpc.invoke_rpc(id,'dinfo',args)
        yield r

        if not r.status():
            print 'database: dinfo_failed',r.args()
            yield async.Coroutine.failure('rpc error')

        result = r.args()[0]
        print 'dinfo_ok',result,logic.is_term(result)

        if result!='None':
            dlist=logic.parse_term(result)
        else:
            dlist=None

        yield async.Coroutine.success(dlist)

    @async.coroutine('internal error')
    def get_icon(self):
        icon_string=self.icon()

        if not icon_string or not icon_string.startswith('ideal(['):
            yield async.Coroutine.success(None)

        print 'browse_db:get_icon:icon_string=',icon_string

        icon=logic.render_term(PC(icon_string, paths.make_subst(self.id()) ))
        result=self.fileCache.get_file(icon)
        yield result

        if not result.status():
            yield async.Coroutine.success(None)

        yield async.Coroutine.success(result.args()[0])
