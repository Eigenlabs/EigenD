
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
from pi import async,paths

class RpcInvocation(async.Deferred,piw.rpcclient):
    def __init__(self,id,path,name,arg,timeout=30000):
        async.Deferred.__init__(self)
        piw.rpcclient.__init__(self)
        arg = piw.makestring_len(arg,len(arg),0)
        name = piw.makestring(name,0)
        piw.tsd_rpcclient(self,id.as_string(),path.make_normal(),name,arg,timeout)
        self.py_lock()

    def rpcclient_complete(self,status,val):
        self.py_unlock()

        if status==0:
            self.failed('timeout/internal error')
            return

        val = val.as_stdstr()
        if status<0:
            self.failed(val)
            return

        self.succeeded(val)

def invoke_rpc(id,name,arg,timeout=30000):
    (addr,path) = paths.breakid(id)
    return RpcInvocation(addr,path,name,arg,timeout)
