
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

from pi import action,agent,atom,domain,files,logic,node,resource,rpc
from plg_simple import icon_manager_version as version
import piw
import picross
import os
import glob

def filesort(a,b):
    return cmp(a[0].lower(),b[0].lower())
    
class Icon(atom.Atom):
    userdir = os.path.join(resource.user_resource_dir('icons',version=''))
    reldir = os.path.join(picross.global_resource_dir(),'icons')

    def __init__(self,agent):    
        print 'Icon: user',self.userdir,'rel',self.reldir
        self.__scan()
        self.agent=agent
        
        atom.Atom.__init__(self,names='icon',protocols='virtual browse',domain=domain.String())

        self.__selection=''

        self.__timestamp = piw.tsd_time()
        self.__update()

    # additional file types?
    #def __scan1(self,pat,pat2,f=lambda x:x):
    def __scan1(self,pat,f=lambda x:x):
        g = lambda path: glob.glob(os.path.join(path,pat)) # + glob.glob(os.path.join(path,pat2))
        paths = g(self.reldir) + g(self.userdir)
        #b = lambda path: os.path.splitext(os.path.basename(path))[0]
        b = lambda path: os.path.basename(path)
        return map(b,paths), map(f,paths)

    def __scan(self):
        self.__values=[]
        #files,paths = self.__scan1('*.[pP][nN][gG]','*.[bB][mM][pP]')
        files,paths = self.__scan1('*.[pP][nN][gG]')
        self.__f2p = dict(zip(files,paths))
        self.__p2f=dict(zip(paths,files))
        self.__files = self.__f2p.keys()
        for f in self.__files:
            self.__values.append((self.__f2p[f],f))
            self.__values.sort(filesort)
        
    def rpc_setselected(self,arg):
        (path,selected)=logic.parse_clause(arg)
        self.__selection=selected
   
    def rpc_activated(self,arg):
        print 'Icon:activated',arg    
        (path,selected)=logic.parse_clause(arg)
        print 'Icon_manager:Icon:activated',path,selected    
        return logic.render_term(('',''))

    def rpc_current(self,arg):
        return '[]'

    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)
        print 'resolving virtual',arg,(a,o)

        if a or not o:
            return '[]'

        o=int(o)

        if o>0 and o<len(self.__values)+1:
            return self.__ideal(self.__values[o-1][0])

        return '[]'

    def rpc_fideal(self,arg):
        (path,uid) = logic.parse_clause(arg)
        return 'ideal([~server,icon],%s)' % logic.render_term(uid)

    def __update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def rpc_enumerate(self,a):
        path = logic.parse_clause(a)
        self.__scan()
        return logic.render_term((len(self.__f2p),0))

    def rpc_cinfo(self,a):
        return '[]'
        #return logic.render_term(())

    def rpc_finfo(self,a):
        (dlist,cnum) = logic.parse_clause(a)
        #map = tuple([(uid,dsc,None,'icon') for (uid,dsc) in self.__values[cnum:]])
        map = tuple([(uid,dsc,None,'icon') for (uid,dsc) in self.__values[cnum:]])
        return logic.render_term(map)
 
    def rpc_auxinfo(self,a):
        print 'auxinfo',a
        (dlist,cnum) = logic.parse_clause(a)
        map = tuple([(dsc,files.get_ideal(logic.make_expansion('~server'),'file_sys:%s'%uid,files.FileSystemFile(uid,dsc),True) ) for (uid,dsc) in self.__values[cnum:]])
        return logic.render_term(map)

    def getIcon(self,a):
        dsc=logic.parse_clause(a)
        print 'Icon Manager getIcon: dsc=',dsc
        if os.path.splitext(dsc)[1]=='':
            dsc=dsc+'.png'
        if dsc in self.__f2p:
            uid=self.__f2p[dsc]
            print 'uid=',uid
            if os.path.exists(uid):
                return logic.render_term((dsc,files.get_ideal(logic.make_expansion('~server'),'file_sys:%s'%uid,files.FileSystemFile(uid,dsc),True))) 

    def resolve_name(self,n):
        if n=='selection':
            return self.__ideals(self.__selection)
        for k,v in self.__values:
            if n==v:
                return self.__ideals(k)
        return self.__ideals()

    def __ideals(self,*cookies):
        return '[%s]' % ','.join([self.__ideal(c) for c in cookies])

    def __ideal(self,cookie):
        print '__ideal',cookie
        return 'ideal([~server,icon],"%s")' % cookie

    def getIconString(self,path):
        if path in self.__p2f:
            return self.__p2f[path]
        # XXX error reporting

class Manager(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self,signature=version,names='icon manager',container=1,ordinal=ordinal)
 
        self.add_verb2(1,'set([],None,role(None,[abstract,matches([icon])]),role(of,[concrete,singular]),role(to,[ideal([~server,icon]),singular])   )',self.__do_icon)
        self[2]=Icon(self)

    def rpc_icon(self,arg):
        return self[2].getIcon(arg)

    def rpc_resolve_ideal(self,arg):
        (type,arg) = action.unmarshal(arg)
        print 'resolving',type,arg

        if type=='icon':
            return self[2].resolve_name(' '.join(arg))

        return action.marshal(())

    def __do_icon(self,a,b,c,d):
        print 'icon_manager:__do_icon',a,b,c,d
        (type,thing) = action.crack_ideal(action.arg_objects(d)[0])
        print 'crack', type,thing
        iconstring=self[2].getIconString(thing)

        id=action.concrete_object(c)
        
        print 'set the icon string on ',id, 'to',iconstring
        rpc.invoke_rpc(id,'set_icon',logic.render_term(iconstring))




agent.main(Manager)
