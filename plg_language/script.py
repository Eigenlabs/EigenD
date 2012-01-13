
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

from pi import atom,logic,const,resource,node,agent,async
import os
import piw
import picross

user_cat = '#User Scripts'
factory_cat = '#Factory Scripts'
response_size = 1200

def render_list(list,offset,renderer):
    txt='['

    for n,l in enumerate(list[offset:]):
        ltxt = '' if not n else ','
        ltxt = ltxt + renderer(n+offset,l)
        if len(txt+ltxt) > response_size-1:
            return txt+']'
        txt=txt+ltxt

    return txt+']'

class ScriptManager(atom.Atom):
    def __init__(self,master_agent,runner):
        self.master_agent = master_agent
        atom.Atom.__init__(self,names='script',protocols='browse')

        self.__factorydir = os.path.join(picross.release_resource_dir(),'Scripts')
        self.__userdir = resource.user_resource_dir(resource.scripts_dir,'')
        self.__timestamp = piw.tsd_time()
        self.__selected=None
        self.__armed=None
        self.__status=None
        self.__runner=runner
        self.update()
        self.__build_cache()

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def __build_cache(self):

        def walker(top_dir,dir_name,fnames):
            for fname in fnames:
                full_name = os.path.join(dir_name,fname)
                x = self.read_script(full_name)

                if not x:
                    continue

                (sn,sd,ss,rn) = x

                if not rn:
                    continue

                self.__cache[rn] = (full_name,os.path.getmtime(full_name))


        self.__cache = {}
        os.path.walk(self.__factorydir,walker,self.__factorydir)
        os.path.walk(self.__userdir,walker,self.__userdir)
        print 'rebuilt cache:',len(self.__cache),'named scripts'


    def read_script(self,filename):
        if not os.path.isfile(filename):
            return None

        f = open(filename,"r")

        description = []
        script = []
        run_name = []

        mode = 'none'
        name = os.path.basename(filename)

        for l in f:

            l=l.strip()

            if l.startswith('#'):
                continue

            if l=='description':
                mode='desc'
                continue

            if l=='script':
                mode='script'
                continue

            if l=='name':
                mode='name'
                continue

            if mode=='desc':
                description.append(l)
                continue

            if mode=='script':
                script.append(l)
                continue

            if mode=='name':
                run_name.extend(l.split())
                continue

        if not description or not script:
            return None

        script = ' '.join(script)
        run_name = ' '.join(run_name)
        
        return (name,description,script,run_name)
            

    def rpc_fideal(self,arg):
        return async.failure('invalid cookie')

    def rpc_displayname(self,arg):
        return 'scripts'

    def rpc_setselected(self,arg):
        (path,selected)=logic.parse_clause(arg)

    def __get_script(self,name):
        x = self.__cache.get(name)

        if x:
            (file_name,file_time) = x
            if os.path.exists(file_name) and os.path.getmtime(file_name)==file_time:
                return file_name

        return None

    def run_script(self,name):
        file_name = self.__get_script(name)

        if not file_name:
            self.__build_cache()
            file_name = self.__get_script(name)

            if not file_name:
                return None

        x = self.read_script(file_name)

        if not x:
            return None

        (sn,sd,sc,rn) = x
        r = self.__runner(file_name,sc)
        return r


    def __run(self):
        s = self.__selected

        x = self.read_script(s)
        if not x:
            self.__status = 'failed'
            self.update()
            return

        (n,d,sc,rn) = x
        self.__status = 'running %s' % n
        self.__armed = None
        self.update()
        r = self.__runner(s,sc)

        def ok(*a,**k):
            if self.__selected==s:
                self.__status = 'succeeded'
                self.update()

        def not_ok(*a,**k):
            if self.__selected==s:
                self.__status = 'failed'
                self.update()

        r.setCallback(ok).setErrback(not_ok)

    def rpc_activated(self,arg):
        (path,selected)=logic.parse_clause(arg)
        if self.__selected == selected and self.__armed == self.__selected:
            self.__run()
            return logic.render_term(('',''))

        self.__selected = selected
        self.__armed = selected
        self.__status='armed'
        print 'arming',self.__armed
        self.update()
        return logic.render_term(('',''))

    def rpc_current(self,arg):
        if self.__selected:
            return logic.render_term(((self.__selected,()),))
        return '[]'

    def rpc_dinfo(self,a):
        path = logic.parse_clause(a)
        l = []
        l.append(('dinfo_id',self.__selected or path))

        if self.__selected:
            x = self.read_script(self.__selected)
            if x:
                (n,d,sc,rn) = x

                if rn:
                    l.append(('name',"%s (%s)"%(n,rn)))
                else:
                    l.append(('name',n))

                l.append(('',''))

                for (i,di) in enumerate(d):
                    l.append(('description' if i==0 else '',di))

                if self.__status:
                    l.append(('',''))
                    l.append(('status',self.__status))

        return logic.render_term(logic.make_term('keyval',tuple(l) ))

    def rpc_enumerate(self, arg):
        path = logic.parse_clause(arg)
        if len(path)>0:
            if path[0]==factory_cat:
                return self.__enumerate_path(self.__factorydir,path[1:])
            if path[0]==user_cat:
                return self.__enumerate_path(self.__userdir,path[1:])

        return logic.render_term((0,2))

    def __enumerate_path(self,root,path):
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

            sfiles += 1

        return logic.render_term((sfiles,sdirs))

    def __cinfo_path(self,root,path):
        dir = os.path.join(root,*path)
        files = os.listdir(dir)
        return [(f,) for f in files if os.path.isdir(os.path.join(dir,f))]

    def __finfo_path(self,root,path):
        dir = os.path.join(root,*path)
        files = os.listdir(dir)
        r = [(os.path.join(dir,f),f,'') for f in files if os.path.isfile(os.path.join(dir,f))]
        return r

    def __finfo0(self,path):
        if len(path)>0:
            if path[0]==factory_cat:
                return self.__finfo_path(self.__factorydir,path[1:])
            if path[0]==user_cat:
                return self.__finfo_path(self.__userdir,path[1:])

        return []

    def rpc_finfo(self,arg):
        (path,idx) = logic.parse_clause(arg)
        return render_list(self.__finfo0(path),idx,lambda i,t: logic.render_term((t[0],str(t[1]),str(t[2]))))

    def __cinfo0(self,path):
        if self.__armed:
            print 'disarming'
            self.__armed=None
            self.__status=None
            self.update()

        if len(path)>0:
            if path[0]==factory_cat:
                return self.__cinfo_path(self.__factorydir,path[1:])
            if path[0]==user_cat:
                return self.__cinfo_path(self.__userdir,path[1:])

        r = []

        if len(path)==0:
            r.insert(0,(factory_cat,))
            r.insert(0,(user_cat,))

        return r

    def rpc_cinfo(self,arg):
        (path,idx) = logic.parse_clause(arg)
        r=self.__cinfo0(path)
        return render_list(r,idx,lambda i,t: logic.render_term((str(t[0]))))

