
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

import wx
import picross
import piw
import random
import optparse
import sys

from pi import atom,action,node,domain,logic,bundles,resource,version,agent,utils
from pisession import gui,session
from pi.logic.shortcuts import T
from pigui import fonts,language,scroller

from app_browser2 import browse,mainframe,history,upgrade

def active():
    print '**************************** active'
    picross.display_active()
    picross.to_front()

class ViewManager(agent.Agent):
    def __init__(self,name):
        agent.Agent.__init__(self,signature=upgrade.Signature(),names='eigenbrowser',volatile=True)

        self.name = name
        self.node = random.randrange(0, 1<<48L) | 0x010000000000L

        self.langModel=gui.call_bg_sync(self.__create_lang)
        self.historyModel=history.HistoryModel(self.langModel)
        self.cdomain = piw.clockdomain_ctl()
        self.cdomain.set_source(piw.makestring('*',0))

        self[25] = scroller.Scroller2(self.__scroll1,names="scroller",ordinal=1) # scrolls the left hand pane
        self[26] = scroller.Scroller(self.__scroll2,self.__tap2,names="scroller",ordinal=2) # scrolls the right hand (dinfo) pane
        self[27] = scroller.Scroller(self.__scroll3,self.__tap3,names="scroller",ordinal=3) # scrolls the path pane

        self.scroller_clone = piw.clone(True)
        self.scroller_clone.set_output(1,self[25].cookie())
        self.scroller_clone.set_output(2,self[26].cookie())
        self.scroller_clone.set_output(3,self[27].cookie())

        self.scroller_in = bundles.VectorInput(self.scroller_clone.cookie(),self.cdomain,signals=(1,2,3,4))

        self[28] = atom.Atom(domain=domain.BoundedFloat(-1,1), init=0, names='roll', policy=self.scroller_in.merge_policy(2,False))
        self[29] = atom.Atom(domain=domain.BoundedFloat(-1,1), init=0, names='yaw', policy=self.scroller_in.merge_policy(1,False))
        self[30] = atom.Atom(domain=domain.BoundedFloat(-1,1), init=0, names='key', policy=self.scroller_in.vector_policy(3,False))
        self[31] = atom.Atom(domain=domain.Aniso(), names='controller', policy=self.scroller_in.merge_policy(4,False))

        self[8]=browse.BrowseAgent(self,browse.getName())

        self.add_verb2(1,'browse([],None,role(None,[or([proto(browse)],[proto(oldbrowse)]),singular]))',callback=self.__do_browse)
        self.add_verb2(2,'minimise([],None)',callback=self.__minimise)
        self.add_verb2(3,'maximise([],None)',callback=self.__maximise)
        
        self.font=wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        self.font.SetPointSize(fonts.DEFAULT_PTS)

        self[12]=atom.Atom(domain=domain.BoundedInt(5,20,rest=11),names='text',protocols='nostage',policy=atom.default_policy(self.__set_fontsize))

        self.__size=node.Server(change=self.__size_changed)
        self.set_private(node.Server())
        self.get_private()[1]=self.__size
        self.__x=100
        self.__y=100
        self.__setup_size()

        self.__minX=60
        self.__minY=80
        self.__maxX=100
        self.__maxY=100

        self.__rootFrame=None
        self.__createRootFrame()

        self[27].reset(-1,1)
        #self[25].set_scheme(1)

        self[25].enable()
        self[26].enable()
        self[27].enable()

    def __do_browse(self,subject,*args):
        print '__do_browse',args
        #active()

        o=''
        if args:
            x=action.arg_objects(args[0])[0]
            if action.is_virtual(x): 
                o = action.crack_virtual(x)
            elif action.is_concrete(x):
                o=action.crack_concrete(x)
            else:
                o=args[0]
            print 'o=',o

        self[8].model.change_target(o)
#
    def __size_changed(self,d):
        print '__size_changed'
        if not d.is_string():
            return False
        l = logic.parse_clause(d.as_string())
        self.__x=l[0]
        self.__y=l[1]
        self.__setup_size()
        if self.__rootFrame:
            self.__doSetSize()

    def __setup_size(self):
        print '__setup_size',self.__x,self.__y
        self.__size.set_data(piw.makestring(logic.render_term((self.__x,self.__y)),0))

    def __create_lang(self):
        return language.LanguageModel()

    def reset1(self,h=None,v=None):
        pass
        #print 'reset1:  v=',v
        #self[25].reset(h,v*-1)
 
    def reset2(self,h=None,v=None):
        self[26].reset(h,v*-1)
    
    def reset3(self,h=None,v=None):
        self[27].reset(h,v*-1)

    def getTitlePanel(self):
        if self.__rootFrame:
            return self.__rootFrame.getTitlePanel()

    def updateStatus(self,text):
        self.__rootFrame.updateStatus(text)

    def doSize(self):
        print 'doSize'
        if self.__rootFrame:
            size=self.__rootFrame.GetSize()
            screenSize=wx.DisplaySize()
            self.__x=int(100*(float(size[0])/float(screenSize[0])))
            self.__y=int(100*(float(size[1])/float(screenSize[1])))
            print 'doSize',size,screenSize,self.__x,self.__y

            self.__setup_size()

    def __set_fontsize(self, value):
        print '__set_fontsize',value
        self.font=wx.Font(value,wx.FONTFAMILY_DEFAULT,wx.FONTSTYLE_NORMAL,weight=wx.FONTWEIGHT_NORMAL)
        return True

    def __scroll1(self,v):
        if v.is_long():
            active()
            vv = v.as_long()
            if (vv&4)!=0: self.__scroll_list(1)
            if (vv&8)!=0: self.__scroll_list(-1)
            if (vv&16)!=0: self.__tap1()

    def __scroll2(self,h,v):
        active()
        self.__scroll_info(h,v*-1)

    def __scroll3(self,h,v):
        active()
        self.__scroll_path(h,v*-1)
    
    def __tap1(self):
        print 'tap 1'
        active()
        vp=self.__getViewPanel()
        if vp:
            vp.tap(1)
    
    def __tap2(self):
        print 'tap 2'
        active()
        vp=self.__getViewPanel()
        if vp:
            vp.tap(2)

    def __tap3(self):
        print 'tap 3'
        active()
        self.onTap3()

    def onTap3(self):
        vp=self.__getPathPanel()
        pathPos=0
        if vp:
            pathPos=vp.getPathPos()
            vp.tap(3)
            self[8].model.setPathPos(pathPos)
  
    def __getViewPanel(self):
        if self.__rootFrame:
            return self.__rootFrame.getViewPanel()
     
    def __getInfoPanel(self):
        if self.__rootFrame:
            return self.__rootFrame.getInfoPanel()
    
    def __getPathPanel(self):
        if self.__rootFrame:
            return self.__rootFrame.getPathPanel()

    def __minimise(self,*args):
        self.__x=self.__minX
        self.__y=self.__minY
        self.__setup_size()
        self.__doSetSize()

    def __doSetSize(self):
        size=wx.DisplaySize()
        size=(  (0.01*self.__x*size[0])-10 , (0.01*self.__y*size[1])-50   )
        self.__rootFrame.SetSize(size)

    def __maximise(self,*args):
        self.__x=self.__maxX
        self.__y=self.__maxY
        self.__setup_size()
        self.__doSetSize()

    def __scroll_list(self,v):
        vp=self.__getViewPanel()
        if vp:
            vp.scroll2(v)
    
    def __scroll_path(self,h,v):
        vp=self.__getPathPanel()
        if vp:
            vp.scroll_both(h,v)

    def __scroll_info(self,h,v):
        vp=self.__getInfoPanel()
        if vp:
            vp.scroll_both(h,v)


    def __createRootFrame(self):
        size=wx.DisplaySize()
        size=(  (size[0]-10),(size[1]-50)   )
        self.__rootFrame=mainframe.MainFrame('',self,size)

        self.__rootFrame.Show()
        self.updateStatus('Connecting...')
        self.__doSetSize()
    
    def doHints(self,hints):
        view,target=logic.parse_clause(hints[0])
        print 'Browser:doHints',view,target
        if view=='browseview':
            self[8].model.change_target(target)
        else:
            print 'No hint'

    def getMainFrame(self):
        return self.__rootFrame

class browserApp(gui.App):
    def __init__(self,name,logfunc):
        gui.App.__init__(self,logfunc=logfunc,name=name)
#        gui.App.__init__(self,name=name)
        print '<browser starting>'
        imageName=resource.find_release_resource('app_browser2','browser_splash.png')
        if imageName:
            image=wx.Image(imageName,wx.BITMAP_TYPE_PNG)
            bmp=image.ConvertToBitmap()
            wx.SplashScreen(bmp,wx.SPLASH_CENTRE_ON_SCREEN|wx.SPLASH_TIMEOUT,2000,None,-1)
            wx.Yield()
        print 'browser starting 2'
        self.agent = ViewManager(name)
        print 'browser starting 3'
        piw.tsd_server(name, self.agent)
        print 'browser starting 4'
        self.agent.advertise('<main>')
        print 'browser starting 5'

def cli():
    parser = optparse.OptionParser()
    parser.add_option('--stdout',action='store_true',dest='stdout',default=False,help='log to stdout')
    picross.pic_init_time()

    x = [ a for a in sys.argv if not a.startswith('-psn') ]
    (opts,args) = parser.parse_args(x)

    name = 'eigenbrowser1'

    lock = resource.LockFile( name )
    if not lock.lock():
        print 'cannot get lock: aborting'
        sys.exit(-1)

    if opts.stdout:
        logfile=sys.__stdout__
    else:
        if sys.platform == 'win32':
            logfile=resource.open_logfile('eigenbrowser')
        else:
            logfile=resource.open_logfile( name )
        
    def logger(msg):
        if logfile:
            print >>logfile,name,msg
            logfile.flush()
    
    try:
        app=browserApp(name,logfunc=logger)
    except:
        import traceback
        print "exception raised" 
        print "start traceback:"
        exeinfo = traceback.format_exc(limit=None)
        print exeinfo
        picross.exit(0)
    
    print "running application MainLoop"
    app.MainLoop()
    picross.exit(0)
