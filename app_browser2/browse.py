
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

from app_browser2 import browse_db
import piw
import picross
from pi import errors,node,logic,action,atom,async,paths,rpc

from pisession import gui

upd_list = 1
upd_info = 2
upd_path = 4
upd_title = 8
upd_all = 15

def getName():
    return 'browseview'

class BrowseSelection(atom.Atom):
    def __init__(self,resolver):
        atom.Atom.__init__(self,names='selection',protocols='virtual')
        self.__resolver = resolver

    def rpc_resolve(self,args):
        self.__resolver(args)

class BrowseAgent(atom.Null):
    def __init__(self,parent,name):
        atom.Null.__init__(self,names=None,container=(None,'browse',parent.verb_container()))
        self.parent=parent
        self.model=BrowseModel(self)
        self.virtual = BrowseSelection(self.model.resolve)
        self.set_property_long('volatile',1)

        self[1]=(self.virtual)
        self.add_verb2(1,'show([],None,role(None,[abstract]))',callback=self.changeDir)
        self.add_verb2(2,'cancel([],None)',callback=self.root)
        self.add_verb2(3,'show([un],None)',callback=self.back)
        self.add_verb2(4,'clear([],None)',callback=self.clear)
        self.__target=node.Server(change=self.__target_changed)
        self.set_private(node.Server())
        self.get_private()[1]=self.__target
        self.__t=''
        self.__setup_target()
 
    def __target_changed(self,d):
        if not d.is_string():
            return False
        id=d.as_string()
        self.__t=id
        self.__setup_target()

        #print 'BrowseAgent.__target_changed:', d.as_string()
        self.model.change_target(id)

    def __setup_target(self):
        self.__target.set_data(piw.makestring(self.__t,0))
     
    def changeDir(self,subject,arg):
        s=action.abstract_string(arg)
        if s in ['dictionary','list','connection','staff','image','conversation']:
            return None
        if 'layer' in s:
            return None
        try:
            dirNum=int(s)-1
        except ValueError:
            return None
        else:
            print 'numCollections=',self.model.numCollections,dirNum
            if (abs(dirNum)>=self.model.numCollections) or dirNum<0:
                return async.success(errors.out_of_range('1 to %s' % str(self.model.numCollections),'show'))
            self.model.changeDir(abs(dirNum))

    def back(self,*args):
        return self.model.back()

    def root(self,*args):
        self.model.goToRoot()

    def clear(self,*args):
        self.setTargetId('')
        self.model.change_target('')

    def getTargetId(self):
        return self.__t

    def setTargetId(self,id):
        self.__t=id
        print 'BrowseAgent: setTargetId to ',self.__t
        self.__setup_target()

class BrowseModel:
    def __init__(self,agent):
        self.agent=agent
        self.proxy=None
        self.new_proxy=None
        self.targetName=''
        self.numFiles=0
        self.numCollections=0
        self.rootNumFiles=0
        self.rootNumCollections=0
        self.current=None
        self.current_categories=()
        self.icon=None
        
        self.fileOffset=0
        self.fileRange=(0,0)
        self.path=[]
        self.cinfo=[]
        self.finfo=[]
        self.keyval=[]
        self.auxDict={}
        self.selected_file=0
        self.category_changed=False
        self.finfo_for_new_category=True
        self.dinfoTarget=''
        self.listListener=None
        self.infoListener=None
        self.titleListener=None
        self.pathListener=None

        self.__updating = False
        self.__updates = 0

    def flush_updates(self):
        f = self.__updates
        self.__updates = 0

        if (f&upd_list): self.listListener.update()
        if (f&upd_info): self.infoListener.update()
        if (f&upd_path): self.pathListener.update()
        if (f&upd_title): self.titleListener.update()

    def add_update(self,flag):
        self.__updates = self.__updates | flag

    def addListListener(self,listener):
        self.listListener=listener

    def addInfoListener(self,listener):
        self.infoListener=listener 

    def addTitleListener(self,listener):
        self.titleListener=listener 

    def addPathListener(self,listener):
        self.pathListener=listener

    def getTargetId(self):
        return self.agent.getTargetId()

    def setTargetId(self,id):
        self.agent.setTargetId(id)

    @async.coroutine()
    def changed(self,id):
        print 'BrowseModel:changed'
        if id==self.getTargetId():
            yield self.__get_name(id)
            yield self.__get_icon()
            if not self.__updating:
                yield self.__get_current(self.getTargetId())
                yield self.__check_enumerate(self.getTargetId(),self.path)
                self.flush_updates()
    
    @async.coroutine()
    def ready(self,id):
        print 'BrowseModel:ready id=',id
        r = gui.defer_bg(self.new_proxy.enumerate,id,[])
        yield r

        if not r.status():
            print 'Enumerate check failed on',id,'- browse target unchanged'
            self.__updating=False
        else:
            print 'Enumerate check suceeded on', id

            db = self.proxy
            if db:
                self.proxy = None
                gui.call_bg_sync(db.removeAllListeners)
                gui.call_bg_sync(db.shutdown)

            self.proxy=self.new_proxy
            self.setTargetId(id)
            self.path=[]
            self.add_update(upd_path)
            yield self.__get_name(id)
            yield self.__get_icon()
            yield self.__get_directory_details(True)
            self.flush_updates()
            self.__updating = False



    def __getBrowserName(self):
        return self.agent.parent.name

    def resolve(self,arg):
        (a,o) = logic.parse_clause(arg)

        print 'resolving virtual',arg,(a,o)

        if not self.getTargetId():
            return '[]'

        if a:
            return '[]'

        if not o:
            o=self.selected_file
        else:
            o=int(o)

        if o<1:
            return '[]'

        print 'resolving',o
        return gui.defer_bg(self.__resolve1,self.getTargetId(),self.path,o,True)
    
    @async.coroutine('internal error')
    def __resolve1(self,id,path,ordinal,asTerm=False):
        a=logic.render_term((tuple(path),ordinal-1))
        r=(yield rpc.invoke_rpc(id,'finfo',a))

        if not r.status():
            print 'rpc error, finfo'
            yield async.Coroutine.failure(*r.args(),**r.kwds())

        flist = logic.parse_clause(r.args()[0])

        if not flist:
            yield async.Coroutine.failure('no selection %d' % ordinal)

        print 'flist=',flist

        (cookie,desc,name)=flist[0][:3]
        if len(flist[0])>3:
            aux=flist[0][3]

        a=logic.render_term((tuple(path),cookie))
        r=(yield rpc.invoke_rpc(id,'fideal',a))

        if not r.status():
            print 'rpc error, fideal'
            yield async.Coroutine.failure(*r.args(),**r.kwds())

        try:
            ideal = logic.parse_clause(r.args()[0],paths.make_subst(id))
        except:
            print 'cant parse:',r.args()[0]
            raise

        print 'converted',ordinal,'to ideal',ideal
        yield async.Coroutine.success(self.__render(ideal,asTerm))

    def __render(self,ideal,asTerm):
        if asTerm:
            return logic.render_term((ideal,)) 
        else:
            return ideal

    def goToRoot(self):
        if self.path:
            self.path=[]
            self.add_update(upd_path)
            self.__get_directory_details(True)
        else:
            return async.success(errors.nothing_to_do('cancel'))

    def back(self):
        if self.path:
            self.path.pop()
            self.add_update(upd_path)
            self.__get_directory_details(True)
        else:
            return async.success(errors.nothing_to_do('show'))

    def changeDir(self,dirNum):
        if self.cinfo and dirNum<=len(self.cinfo):
            dirIndex=dirNum
            dirName=self.cinfo[dirIndex][1]
            self.path.append(dirName)
            self.add_update(upd_path)
            self.__get_directory_details(True)

    def setPathPos(self,pos):
        if self.path:
            self.path=self.path[:pos]
            self.add_update(upd_path)
            self.__get_directory_details(True)

    @async.coroutine('internal error')
    def change_target(self,targetId):
        self.__pending = targetId

        while True:
            if self.__updating:
                yield async.Coroutine.success()

            pending = self.__pending
            self.__pending = None

            if pending is None:
                yield async.Coroutine.success()

            yield self.__change_target(pending)

    @async.coroutine('internal error')
    def __change_target(self,targetId):
        print 'browseModel.change_target',targetId

        self.__updating = True

        if targetId=='':
            if self.agent.getTargetId()=='':
                self.targetName=''
                self.path=[]
                self.cinfo=[]
                self.finfo=[]
                self.keyval=[]
                self.numFiles=0
                self.numCollections=0
                self.icon=None
                self.add_update(upd_all)
                self.flush_updates()
                self.__updating = False
                yield async.Coroutine.success()

            targetId = self.agent.getTargetId()

        print 'BrowseModel: Attempt target change to',targetId,'from',self.getTargetId()

        if self.proxy is None or (targetId !=self.getTargetId()):
            self.new_proxy=gui.call_bg_sync(self.__create_browse_proxy,targetId)
            # proxy calls ready() when its ready
        else:
            self.path=[]
            yield self.__get_directory_details(True)
            self.__updating = False

    def __create_browse_proxy(self,id):
        bp=browse_db.BrowseProxy(id,self.__getBrowserName(),self)
        return bp

    @async.coroutine('internal error')
    def __get_name(self,targetId):    
        if self.proxy is None:
            print '__get_name: proxy is None - returning'
            return
            
        db=self.proxy
        r=gui.defer_bg(db.getName,targetId)
        yield r
        self.targetName = ''
        if r.status():
            (self.targetName,) = r.args()
        self.add_update(upd_title)
        print 'get_name',self.targetName

    @async.coroutine('internal error')
    def __get_directory_details(self,flush):
        print 'getDirectoryDetails'
        self.fileOffset=0
        self.listListener.set_refresh()
        self.listListener.set_selected(0)
        self.cinfo=[]
        yield self.__get_current(self.getTargetId())
        yield self.__enumerate(self.getTargetId(),self.path)
        yield self.__get_cinfo(self.getTargetId(),self.path,0,10000)
        yield self.__get_dinfo(self.getTargetId(),self.path)
        print 'getDirectoryDetails updating'
        if flush:
            self.flush_updates()
        print 'getDirectoryDetails done'

    @async.coroutine('internal error')
    def __get_cinfo(self,id,path,start,ncolls):
        if self.proxy is None:
            return

        r = gui.defer_bg(self.proxy.cinfo,id,path,start,ncolls)

        yield r

        if not r.status():
            print 'browse get_cinfo:notok',r.args()
            self.cinfo=[]
            count=0
            self.add_update(upd_list)
            yield async.Coroutine.success()

        (colls,total) = r.args()

        cinfo=colls
        cinfo.sort()
        self.cinfo=[]
        count=0

        if path:
            self.cinfo.append(('!control'+str(count),'Up one level'))
            self.numCollections=self.numCollections+1 
            count=count+1

        for c in cinfo:
            self.cinfo.append(('!collection'+str(count),c))
            count=count+1
        
        self.category_changed=True
        print 'get_cinfo ok'
        yield self.__get_finfo(self.getTargetId(),self.path)

    @async.coroutine('internal error')
    def __get_dinfo(self,id,path):
        if self.proxy is None:
            return

        r = gui.defer_bg(self.proxy.dinfo,id,path)

        yield r

        if not r.status():
            print 'dinfo failed',r.args()
            #picross.to_front()
            self.keyval=[]
            self.add_update(upd_info)
            yield async.Coroutine.success()

        (dlist,) = r.args()

        print 'got_dinfo',dlist,logic.is_term(dlist)
        #picross.to_front()
        self.keyval=[]
        if logic.is_term(dlist):
            if logic.is_pred(dlist,'keyval'):
                dinfo=dlist.args[0]
                if self.dinfoTarget!=dinfo[0][1]:
                    self.dinfoTarget=dinfo[0][1]
                    if self.infoListener:
                        self.infoListener.targetChanged()
                
                if len(dinfo)>1:
                    for k,v in dinfo[1:]:
                        print k,v
                        self.keyval.append((k,v))

            self.add_update(upd_info)


    @async.coroutine('internal error')
    def __get_finfo(self,id,path):
        if self.proxy is None:
            return

        r = gui.defer_bg(self.proxy.finfo,id,path)

        yield r

        if not r.status():
            print 'browse get_finfo:notok',r.args()
            self.finfo=[]
            self.numFiles=len(self.finfo)
            self.finfo_for_new_category=False
            self.add_update(upd_list)
            yield async.Coroutine.success()

        (files,total) = r.args()

        resultlist=files
        if self.finfo!=resultlist: 
            self.finfo=resultlist
            self.numFiles=len(self.finfo)
            self.add_update(upd_list)
        
        if self.category_changed:
            self.category_changed=False
            self.finfo_for_new_category=True
            self.add_update(upd_list)

        print 'browse get_finfo:ok'


    @async.coroutine('internal error')
    def __enumerate(self,id,path):
        if self.proxy is None:
            return

        r = gui.defer_bg(self.proxy.enumerate,id,path)

        yield r

        if not r.status():
            print 'browse enumerate:notok',r.args()
            yield async.Coroutine.success()

        (numFiles,numColl) = r.args()

        self.numFiles=numFiles
        self.numCollections=numColl

        if self.path==[]:
            self.rootNumFiles=self.numFiles
            self.rootNumCollections=self.numCollections

        self.listListener.resetVScroller(1)
        self.listListener.offset=0



    @async.coroutine('internal error')
    def __get_current(self,id):
        if self.proxy is None:
            return

        r = gui.defer_bg(self.proxy.current,id)

        yield r

        if not r.status():
            print 'browse current:notok',r.args()
            yield async.Coroutine.success()

        (current,) = r.args()

        if current:
            (cookie,categories)=current[0]
            print 'current ok',cookie,categories
            if self.current !=cookie:
                self.current=cookie
                self.add_update(upd_list)
            if self.current_categories!=categories:
                self.current_categories=categories
                self.add_update(upd_list)
        else:
            self.current=None
            self.current_categories=()
            self.add_update(upd_list)


    @async.coroutine('internal error')
    def __check_enumerate(self,id,path):
        if self.proxy is None:
            return

        r = gui.defer_bg(self.proxy.enumerate,id,path)

        yield r

        if not r.status():
            print 'browse check_enumerate:notok',arg
            yield async.Coroutine.success()

        (numFiles,numColl) = r.args()

        if numFiles==self.numFiles:
            print 'check_enumerate ok:'
            yield self.__get_finfo(self.getTargetId(),path)
            yield self.__get_dinfo(self.getTargetId(),path)
        else:
            print 'check_enumerate ok: number of files',numFiles,'changed'
            self.path=[]
            yield self.__get_directory_details(False)

    @async.coroutine('internal error')
    def __get_icon(self):
        if self.proxy is None:
            return

        r = gui.defer_bg(self.proxy.get_icon)
        yield r
        self.icon=None
        if r.status():
            (self.icon,) = r.args()
        self.add_update(upd_list)

    def activate(self,selection):
        print 'activate',selection
        if selection>0 and selection<=len(self.finfo):
            uid=self.finfo[selection-1][0] 
            print 'Activate: No. in browser list=',selection,'uid=',uid
            
            args=logic.render_term((tuple(self.path),uid))
            self.__do_activate(self.getTargetId(),args)

    @async.coroutine('internal error')
    def __do_activate(self,id,args):
        print '__do_activate',id,args
        r=rpc.invoke_rpc(id,'activated',args)
        yield r
        if r.status():
            self.agent.parent.doHints(r.args())

    def setSelectedFile(self,selected):
        if selected>0 and selected<=len(self.finfo):
            #print 'browseModel:setSelectedFile: No. in browser list=',selected,'uid=',self.finfo[selected-1]
            self.selected_file=self.finfo[selected-1][0] 
            args=logic.render_term((tuple(self.path),self.selected_file))
            rpc.invoke_rpc(self.getTargetId(),'setselected',args)
    
    def getPath(self):
        if self.targetName:
            path=self.path
            if self.numFiles==0 and self.numCollections==0:
                path=['No Path']
            return path
        else:
            return ['No Path']

