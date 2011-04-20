
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

import sqlite3
import os
import loop_native
import picross

from pi import resource

user_cat = '#User Loops (by file)'
factory_cat = '#Factory Loops (by file)'

def quotelist(a):
    return ','.join(["'%s'"%x for x in a])

def qfilelist(a):
    q ="""select id,count(id) as idcount from meta where tag in (%s) group by id""" % quotelist(a)
    return q

def qfilecount(a):
    q = """select count(*) from (%s) where idcount=%d""" % (qfilelist(a),len(a))
    return q

def qdirlist(a):
    if len(a)==0:
        return """select distinct tag from meta"""

    wheres = []
    for w in a:
        wheres.append("""id in (select id from meta where tag='%s')""" % w)

    if wheres:
        wheres = ' where ' + ' and '.join(wheres)

    q = """
        select distinct t1 from (
            select id,tag as t1 from meta where tag in (
                select distinct tag from meta where tag not in (%s)
            ) group by id
        )
        %s""" % (quotelist(a), wheres)
    return q

def qdircount(a):
    q = """select count(*) from (%s)""" % (qdirlist(a))
    return q

def qcinfo(a):
    return qdirlist(a)

def qfmatch(a):
    q = """select id from (%s) where idcount=%d""" % (qfilelist(a),len(a))
    return q

def qpinfo(a):
    q = """select id,desc,name from files where file like '%s%s%%'""" % (a,os.path.sep)
    return q

def qfinfo(a):
    q = """select id,desc,name from files where id in (%s)""" % qfmatch(a)
    return q

def qmodtime():
    q = """select id,time from modtime"""
    return q

def qtables():
    q= """select name from sqlite_master where type='table'"""
    return q

class LoopDatabase:
    def __init__(self):
        self.__dbfile = os.path.join(resource.user_resource_dir('Loop'),'loops.db')
        self.__factorydir = os.path.join(picross.global_resource_dir(),'loop')
        self.__userdir = resource.user_resource_dir('Loop','')
        e = os.path.exists(self.__dbfile)
        if not e:
            self.scan()
        else:
            self.__check_modtime()
        self.prime()

    def scan(self,mtime1=None,mtime2=None):
        print 'building database...'
        cx = sqlite3.connect(self.__dbfile)
        cx.execute("""create table files (id int primary key,name text,desc text,file text, basename text)""")
        cx.execute("""create table meta (id int,tag text)""")
        cx.execute("""create table modtime (id int, time int)""")

        files = []
        meta = []
        nxt = 0
        nxt = self.__scandir(nxt,files,meta,os.path.join(picross.global_resource_dir(),'loop'))
        nxt = self.__scandir(nxt,files,meta,resource.user_resource_dir('Loop',''))
        print '%d loops indexed' % nxt

        if not mtime1:
            mtime1=self.__get_modtime(resource.user_resource_dir('Loop',''))

        if not mtime2:
            mtime2=self.__get_modtime(os.path.join(picross.global_resource_dir(),'loop'))

        modtime=[(1,mtime1),(2,mtime2)]

        cx.executemany("""insert into modtime(id,time) values (?,?)""",modtime)
        cx.executemany("""insert into files(id,desc,file,basename) values (?,?,?,?)""",files)
        cx.executemany("""insert into meta(id,tag) values (?,?)""",meta)
        cx.execute("""create index mx on meta (tag)""")
        cx.commit()

    def rescan(self):
        print 'rebuild loop database by request'
        os.remove(self.__dbfile)
        self.scan()
        self.prime()

    def __get_modtime(self,dir):
        mtime = 0

        for (p,d,f) in os.walk(dir):
            mtime1=int(os.path.getmtime(p))
            if mtime1 > mtime: mtime=mtime1

        return mtime

    def __check_modtime(self):
        mtime1=self.__get_modtime(resource.user_resource_dir('Loop',''))
        mtime2=self.__get_modtime(os.path.join(picross.global_resource_dir(),'loop'))
        modtime=self.__modtime()
        if not modtime or (abs(modtime[0][1]-mtime1)>1) or (abs(modtime[1][1]-mtime2)>1):
            print 'LoopDatabase:database needs rebuilding',modtime,mtime1,mtime2
            os.remove(self.__dbfile)
            self.scan(mtime1=mtime1,mtime2=mtime2)

    def __scandir(self,n,frows,mrows,path):
        for root,dirs,files in os.walk(path):
            for f in files:
                try:
                    if f.startswith('.') or f.startswith('_'):
                        continue
                    (b,e) = os.path.splitext(f)
                    if e.lower()=='.aiff' or e.lower()=='.aif':
                        p = os.path.join(root,f)
                        tags = self.__getmeta(p)
                        frows.append((n,b,p,f))
                        mrows.extend([(n,t) for t in tags])
                        n = n+1
                        if n%500==0: print '%d loops indexed'%n
                except:
                    pass
        return n

    def __getmeta(self,file):
        lf = loop_native.read_aiff(file,True)
        return [lf.tag(i) for i in range(0,lf.ntags())]

    def __modtime(self):
        cx = sqlite3.connect(self.__dbfile)
        tables = cx.execute(qtables()).fetchall()
        for t in tables:
            if 'modtime' in t:
                r = cx.execute(qmodtime()).fetchall()
                return r

        print 'modtime not in tables'
        return 0
   
    def enumerate(self, path):
        self.__check_modtime()

        if len(path)>0:
            if path[0]==factory_cat:
                return self.enumerate_path(self.__factorydir,path[1:])
            if path[0]==user_cat:
                return self.enumerate_path(self.__userdir,path[1:])

        cx = sqlite3.connect(self.__dbfile)
        nf = cx.execute(qfilecount(path)).fetchone()[0]
        nc = cx.execute(qdircount(path)).fetchone()[0]

        if len(path)==0:
            nc=nc+2
        return (nf,nc)

    def enumerate_path(self,root,path):
        dir = os.path.join(root,*path)
        sdirs = 0
        sfiles = 0

        for f in os.listdir(dir):
            ff = os.path.join(dir,f)

            if os.path.isdir(ff):
                sdirs += 1
                continue

            if not os.path.isfile(ff):
                continue

            fl = f.lower()
            if fl.endswith('.aif') or fl.endswith('.aiff'):
                sfiles += 1
                continue

        return (sfiles,sdirs)

    def cinfo_path(self,root,path):
        dir = os.path.join(root,*path)
        files = os.listdir(dir)
        return [(f,) for f in files if os.path.isdir(os.path.join(dir,f))]

    def finfo_path(self,root,path):
        dir = os.path.join(root,*path)
        cx = sqlite3.connect(self.__dbfile)
        r = cx.execute(qpinfo(dir)).fetchall()
        return r

    def cinfo(self,path):
        if len(path)>0:
            if path[0]==factory_cat:
                return self.cinfo_path(self.__factorydir,path[1:])
            if path[0]==user_cat:
                return self.cinfo_path(self.__userdir,path[1:])

        cx = sqlite3.connect(self.__dbfile)
        r = cx.execute(qcinfo(path)).fetchall()
        if len(path)==0:
            r.insert(0,(factory_cat,))
            r.insert(0,(user_cat,))
        return r

    def finfo(self,path):
        if len(path)>0:
            if path[0]==factory_cat:
                return self.finfo_path(self.__factorydir,path[1:])
            if path[0]==user_cat:
                return self.finfo_path(self.__userdir,path[1:])
        cx = sqlite3.connect(self.__dbfile)
        r = cx.execute(qfinfo(path)).fetchall()
        return r

    def rename(self,id,name):
        cx = sqlite3.connect(self.__dbfile)
        c = cx.cursor()
        c.execute("""update files set name=? where id=?""", (str(name),int(id)))
        c.close()
        cx.commit()

    def size(self):
        cx = sqlite3.connect(self.__dbfile)
        nf = cx.execute("""select count(*) from files""").fetchone()[0]
        return nf

    def describe(self,id):
        cx = sqlite3.connect(self.__dbfile)
        r = cx.execute("""select name,desc from files where id=?""",  (id,)).fetchone()
        name = str(r[0]) if r[0] else None
        desc = str(r[1]) if r[1] else None
        return name,desc

    def file(self,id):
        cx = sqlite3.connect(self.__dbfile)
        r = cx.execute("""select file from files where id=?""",  (id,)).fetchone()
        if r:
            return str(r[0])

    def basename(self,id):
        cx = sqlite3.connect(self.__dbfile)
        r = cx.execute("""select basename from files where id=?""",  (id,)).fetchone()
        if r:
            return str(r[0])

    def id(self,name):
        cx = sqlite3.connect(self.__dbfile)
        r = cx.execute("""select id from files where name=?""",  (name,)).fetchone()
        if r:
            return r[0]
    
    def idforfile(self,file):
        cx = sqlite3.connect(self.__dbfile)
        r = cx.execute("""select id from files where file=?""",  (file,)).fetchone()
        if r:
            return r[0]
    
    def idforbasename(self,basename):
        cx = sqlite3.connect(self.__dbfile)
        r = cx.execute("""select id from files where basename=?""",  (basename,)).fetchone()
        if r:
            return r[0]
    
    def fileforbasename(self,basename):
        cx = sqlite3.connect(self.__dbfile)
        r = cx.execute("""select file,id from files where basename=?""",  (basename,)).fetchone()
        if r: return r
        return (None,None)

    def tags(self,id):
        cx = sqlite3.connect(self.__dbfile)
        r = cx.execute("""select tag from meta where id=?""",  (id,)).fetchall()
        return [str(a[0]) for a in r]



    def prime(self):
        self.size()

def create():
    db = LoopDatabase()

