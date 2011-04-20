
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

import struct
import operator

class RiffError(RuntimeError):
    pass

class Chunk:
    def __init__(self,file,name):
        self.file = file
        self.name = name
        self.id = self.file.read(4)
        if len(self.id)<4:
            self.eof = True
            self.remain = 0
        else:
            self.remain = struct.unpack('<L',self.file.read(4))[0]
            if self.id in ('RIFF','LIST'):
                self.id = self.read(4)

    def read(self,n):
        if self.remain<n:
            raise RiffError()
        self.remain -= n
        return self.file.read(n)

    def read_external(self,reader):
        r = reader(self.file,self.remain)
        self.remain = 0
        return r

    def read_indirect(self,reader):
        r = reader(self.name,self.file.tell(),self.remain)
        self.file.seek(self.remain,1)
        self.remain = 0
        return r

    def read_ntbs(self,len):
        value = ''
        for i in range(0,len):
            ch = self.read(1)
            if ord(ch)==0:
                return value
            value += ch
        raise RiffError()

    def read_chunk(self):
        if self.remain<8:
            return None
        subc = Chunk(self.file,self.name)
        self.remain = self.remain-(subc.remain+8)
        return subc

    def skip(self):
        if self.remain>0:
            self.file.seek(self.remain,1)
            self.remain = 0

class List:
    def __init__(self,**bits):
        self.__bits = bits

    def read(self,c):
        ret = {}
        subc = c.read_chunk()
        while subc is not None:
            if subc.id in self.__bits:
                ret[subc.id]=self.__bits[subc.id].read(subc)
            subc.skip()
            subc = c.read_chunk()

        return ret

class String:
    def __init__(self,len=256):
        self.__len = len
    def read(self,c):
        return c.read_ntbs(self.__len)

class Struct:
    def __init__(self,fmt):
        self.__fmt = fmt
        self.__size = struct.calcsize(fmt)
    def read(self,c):
        return struct.unpack(self.__fmt,c.read(self.__size))

class StructList:
    def __init__(self,fmt):
        self.__fmt = fmt
        self.__size = struct.calcsize(fmt)
    def read(self,c):
        s = []
        while c.remain>=self.__size:
            raw = c.read(self.__size)
            s.append(struct.unpack(self.__fmt,raw))
        return s

class Root:
    def __init__(self,rname,root):
        self.__rname = rname
        self.__root = root

    def read(self,file,name=None):
        c = Chunk(file,name)
        if c.id==self.__rname:
            return self.__root.read(c)
        return None

