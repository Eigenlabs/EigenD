
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

from pi import logic, async, rpc
from pi import resource as pi_resource
from picross import resource

import hashlib
import os

def get_ideal(server,cookie,file,perm):
    return logic.render_term(get_ideal_term(server,cookie,file,perm))

def get_ideal_term(server,cookie,file,perm):
    return logic.make_term('ideal',(server,'file'), (perm,(file.label(),file.type(),file.size(),file.md5()),cookie))

class LiteralFile:
    def __init__(self,label,type,contents):
        self.__contents = contents
        self.__type = type
        self.__label = label

    def size(self):
        return len(self.__contents)

    def data(self,offset,length):
        return self.__contents[offset:offset+length]

    def label(self):
        return self.__label

    def type(self):
        return self.__type

    def md5(self):
        return hashlib.md5(self.__contents).hexdigest()

class ResourceFile(LiteralFile):
    def __init__(self,name):
        type = os.path.splitext(name).strip('.')
        contents = resource.get_resource(name,level=2)
        LiteralFile.__init__(self,name,type,contents)

class PkgResourceFile(LiteralFile):
    def __init__(self,name):
        bits = name.split('/')
        pkg = '.'.join(bits[:-1])
        file = bits[-1]
        type = os.path.splitext(file)[1].strip('.')
        contents = resource.get_pkg_resource(pkg,file)
        LiteralFile.__init__(self,file,type,contents)

class FileSystemFile:
    def __init__(self,name,label):
        self.__name = name
        self.__hash = hashlib.md5(pi_resource.file_open(name,'r').read()).hexdigest()
        self.__size = pi_resource.os_path_getsize(name)
        self.__label = label
        print 'file system file, name=',name,'hash=',self.__hash

    def label(self):
        return self.__label

    def size(self):
        return self.__size

    def data(self,offset,length):
        f = pi_resource.file_open(self.__name,'rb')
        f.seek(offset,0)
        if offset+length > self.__size:
            length = self.__size - offset
        return f.read(length)

    def type(self):
        return 'take'

    def md5(self):
        return self.__hash

transfer_size = 7000

@async.coroutine('internal error')
def get_data(ideal,cache=None):
    t = logic.parse_clause(ideal)

    if not logic.is_pred_arity(t,'ideal',2,2) or not logic.is_list(t.args[0]) or t.args[0][1] != 'file':
        yield async.Coroutine.failure('%s not a file reference' % ideal)
        return


    server = t.args[0][0]
    (perm,(label,type,size,md5),(server,cookie)) = t.args[1]

    print 'files.get_data:',server,':',cookie
    data = ''
    hash = hashlib.md5()

    while True:
        fetched = len(data)
        remaining = size-fetched

        if remaining <= 0:
            break

        if remaining > transfer_size:
            remaining = transfer_size

        x = logic.render_term((cookie,fetched,remaining))
        result = rpc.invoke_rpc(server,'download',x)
        yield result

        if not result.status():
            yield async.Coroutine.failure('file transfer error: %s' % result.args()[0])
            return

        rsp = result.args()[0]
        if len(rsp) < remaining:
            yield async.Coroutine.failure('file transfer error: short read')
            return
            

        hash.update(rsp)
        data += rsp

    if hash.hexdigest() == md5:
        yield async.Coroutine.success(data)

    yield async.Coroutine.failure('file transfer error: checksum error')

class FileCache:
    def __init__(self,cache):
        self.__cache_dir = pi_resource.cache_dir()
        self.__cache_dir = os.path.join(self.__cache_dir,cache)
        self.__waiters = {}

        try:
            pi_resource.os_makedirs(self.__cache_dir)
        except:
            pass

    def get_file(self,ideal):
        t = logic.parse_clause(ideal)

        if not logic.is_pred_arity(t,'ideal',2,2) or not logic.is_list(t.args[0]) or t.args[0][1] != 'file':
            return async.failure('%s not a file reference' % ideal)

        server = t.args[0][0]
        (perm,(label,type,size,md5),cookie) = t.args[1]

        uuid = '%s-%d-%s.%s' % (label,size,md5,type)

        r = async.Deferred()

        if uuid in self.__waiters:
            print 'tagging onto',uuid,'download'
            self.__waiters[uuid].append(r)
            return r

        self.__waiters[uuid] = [r]

        def ok(*args,**kwds):
            w = self.__waiters[uuid]
            del self.__waiters[uuid]
            for ww in w: ww.succeeded(*args,**kwds)

        def nok(*args,**kwds):
            w = self.__waiters[uuid]
            del self.__waiters[uuid]
            for ww in w: ww.failed(*args,**kwds)

        cache_file = os.path.join(self.__cache_dir,uuid)

        r2 = self.__getfile(cache_file,size,server,cookie,md5)
        r2.setCallback(ok).setErrback(nok)

        return r

    @async.coroutine('internal error')
    def __getfile(self,cache_file,size,server,cookie,md5):

        if pi_resource.os_path_exists(cache_file) and pi_resource.os_path_getsize(cache_file)==size:
            print 'returning',server,':',cookie,'from cache'
            yield async.Coroutine.success(cache_file)

        print 'FileCache.__getfile:downloading',server,':',cookie,'->',cache_file

        hash = hashlib.md5()
        data = pi_resource.file_file(cache_file,"w")
        fetched = 0

        while True:
            remaining = size-fetched
            print 'FileCache.__getfile:',server,cookie,'size',size,'fetched',fetched,'transfer_size',transfer_size

            if remaining <= 0:
                break

            if remaining > transfer_size:
                remaining = transfer_size

            x = logic.render_term((cookie,fetched,remaining))
            print 'x=',x
            result = rpc.invoke_rpc(server,'download',x)
            yield result

            if not result.status():
                yield async.Coroutine.failure('file transfer error: %s' % result.args()[0])
                return

            rsp = result.args()[0]
            if len(rsp) < remaining:
                yield async.Coroutine.failure('file transfer error: short read')
                return
                

            hash.update(rsp)
            fetched += len(rsp)
            data.write(rsp)

        data.close()

        if hash.hexdigest() == md5:
            yield async.Coroutine.success(cache_file)

        yield async.Coroutine.failure('file transfer error: checksum error')

@async.coroutine('internal error')
def copy_file(ideal,filename):
    t = logic.parse_clause(ideal)

    if not logic.is_pred_arity(t,'ideal',2,2) or not logic.is_list(t.args[0]) or t.args[0][1] != 'file':
        yield async.Coroutine.failure('%s not a file reference' % filename)
        return

    server = t.args[0][0]
    (perm,(label,type,size,md5),cookie) = t.args[1]

    print 'downloading',ideal,'to',filename

    hash = hashlib.md5()
    data = pi_resource.file_file(filename,"w")
    fetched = 0

    while True:
        remaining = size-fetched

        if remaining <= 0:
            break

        if remaining > transfer_size:
            remaining = transfer_size

        x = logic.render_term((cookie,fetched,remaining))
        result = rpc.invoke_rpc(server,'download',x)
        yield result

        if not result.status():
            pi_resource.os_unlink(filename)
            yield async.Coroutine.failure('file transfer error: %s' % result.args()[0])
            return

        rsp = result.args()[0]
        if len(rsp) < remaining:
            pi_resource.os_unlink(filename)
            yield async.Coroutine.failure('file transfer error: short read')
            return

        hash.update(rsp)
        fetched += len(rsp)
        data.write(rsp)

    data.close()
    print 'size',size,'fetched',fetched,'transfer_size',transfer_size

    if hash.hexdigest() == md5:
        yield async.Coroutine.success(filename)

    pi_resource.os_unlink(filename)
    yield async.Coroutine.failure('file transfer error: checksum error')
