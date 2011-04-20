
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

import eigend_native

from pi import agent,resource,utils
from pibelcanto import translate
from app_eigend2 import version
from pisession import agentd,session,upgrade

import bugs

import piw
import picross
import os
import sys
import time
import threading
import gc

class EigenOpts:
    def __init__(self):
        self.stdout = False
        self.noauto = False
        self.noredirect = False


class GarbageCollector(threading.Thread):
    def __init__(self,scaffold,context):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self.context = context
        self.scaffold = scaffold
        self.event1 = threading.Event()
        self.event1.clear()
        self.running = True
        self.delay = 10

    def stop_gc(self):
        self.running = False

    def start_gc(self):
        self.running = True
        self.event1.set()

    def run_pass(self,p):
        if self.scaffold.global_lock():
            print 'starting gc pass',p
            try:
                o = gc.collect(p)
                if o: print 'gc collected',o
            except:
                pass
            print 'finished gc pass',p
            self.scaffold.global_unlock()

    def passes(self):
        while True:
            for i in xrange(0,10): yield 0
            for i in xrange(0,10): yield 1
            yield 2


    def run(self):
        piw.setenv(self.context.getenv())
        gc.disable()

        while True:
            for p in self.passes():
                self.event1.wait(self.delay)
                self.event1.clear()
                if not self.running:
                    break
                self.run_pass(p)


class Agent(agentd.Agent):
    def __init__(self,backend):
        self.backend = backend
        path = [os.path.join(picross.release_root_dir(),'plugins')]
        agentd.Agent.__init__(self,1,path)
        piw.tsd_server('<eigend1>',self)

    def stop_gc(self):
        self.backend.stop_gc()

    def start_gc(self):
        self.backend.start_gc()

    def load_started(self,setup):
        self.backend.load_started(setup)

    def load_ended(self):
        self.backend.load_ended()

    def load_status(self,message,progress):
        self.backend.load_status(message,progress)

    def load_complete(self,errors=[]):
        self.backend.load_ended()
        if errors:
            self.backend.alert_dialog('Load Problems','Load Problems','\n'.join(errors))

    def create_context(self,name,callback,gui):
        return self.backend.create_context(name,callback,gui)

    def run_in_gui(self,func,*args,**kwds):
        return self.backend.run_foreground_sync(func,*args,**kwds)

    def setups_changed(self,file=None):
        self.backend.setups_changed(file)

    def set_default_setup(self,path):
        self.backend.set_default_setup(path)


class Backend(eigend_native.c2p):
    def __init__(self):
        eigend_native.c2p.__init__(self)
        self.bugfiler = bugs.BugFiler(self)
        self.collector = None
        self.__progress = None
        self.__progress_lock = threading.Lock()
        self.saving = False
        self.quitting = False
        self.savcond = threading.Condition()

        self.fgthing = piw.thing()
        self.fgthing.set_slow_trigger_handler(utils.notify(self.fgdequeue))
        self.fgqueue = []

        self.bgthing = piw.thing()
        self.bgthing.set_slow_trigger_handler(utils.notify(self.bgdequeue))
        self.bgqueue = []

        upgrade.clr_tmp_setup()

    def stop_gc(self):
        if self.collector is not None:
            print 'stopping gc'
            self.collector.stop_gc()

    def start_gc(self):
        if self.collector is not None:
            print 'restarting gc'
            self.collector.start_gc()

    def set_args(self,argv):
        args = argv.split()

        self.opts = EigenOpts()

        if '--stdout' in args:
            args.remove('--stdout')
            self.opts.stdout = True

        if '--noauto' in args:
            args.remove('--noauto')
            self.opts.noauto = True

        if '--noredirect' in args:
            args.remove('--noredirect')
            self.opts.stdout = True
            self.opts.noredirect = True

        self.args = args

    def __load_started(self,setup):
        self.frontend.load_started(setup)

    def __load_ended(self):
        self.frontend.load_ended()

    def __set_latest_release(self,release):
        self.frontend.set_latest_release(release)

    def __create_context(self,name,callback,gui):
        logger = self.frontend.make_logger(name)
        if gui:
            return self.scaffold.context(utils.statusify(callback),logger,name)
        else:
            return self.scaffold.bgcontext(utils.statusify(callback),logger,name)

    def load_started(self,setup):
        self.run_foreground_async(self.__load_started,setup)

    def load_ended(self):
        self.run_foreground_async(self.__load_ended)

    def __load_status(self):
        old = None

        self.__progress_lock.acquire()
        try:
            old = self.__progress
            self.__progress = None
        finally:
            self.__progress_lock.release()

        if old:
            self.frontend.load_status(*old)

    def load_status(self,message,progress):
        self.__progress_lock.acquire()
        old = None

        try:
            old = self.__progress
            self.__progress = (message,progress)

        finally:
            self.__progress_lock.release()

        if not old:
            self.run_foreground_async(self.__load_status)

    def set_latest_release(self,release):
        self.run_foreground_async(self.__set_latest_release,release)

    def create_context(self,name,callback,gui):
        return self.run_foreground_sync(self.__create_context,name,callback,gui)

    def run_background(self,func,*args,**kwds):
        current_context = piw.tsd_snapshot()

        try:
            piw.setenv(self.backend_context.getenv())
            piw.tsd_lock()
            try:
                return func(*args,**kwds)
            finally:
                piw.tsd_unlock()

        finally:
            current_context.install()

    def run_background_async(self,func,*args,**kwds):
        self.bgqueue.append((func,args,kwds,None))
        self.bgthing.trigger_slow()

    def bgdequeue(self):
        while self.bgqueue:
            (f,a,k,n) = self.bgqueue[0]
            self.bgqueue = self.bgqueue[1:]
            status = False
            r = None
            try:
                try:
                    r = f(*a,**k)
                    status = True
                except:
                    utils.log_exception()
            finally:
                try:
                    if n:
                        n(status,r)
                except:
                    utils.log_exception()

    def run_foreground_async(self,func,*args,**kwds):
        self.fgqueue.append((func,args,kwds,None))
        self.fgthing.trigger_slow()

    def fgdequeue(self):
        while self.fgqueue:
            (f,a,k,n) = self.fgqueue[0]
            self.fgqueue = self.fgqueue[1:]
            status = False
            r = None
            try:
                try:
                    r = f(*a,**k)
                    status = True
                except:
                    utils.log_exception()
            finally:
                try:
                    if n:
                        n(status,r)
                except:
                    utils.log_exception()

    def run_foreground_sync(self,func,*args,**kwds):
        event = threading.Event()
        event.clear()
        retv = [None,None]

        def handler(status,v):
            retv[0] = status
            retv[1] = v
            event.set()

        self.fgqueue.append((func,args,kwds,handler))
        self.fgthing.trigger_slow()

        event.wait()

        if retv[0]:
            return retv[1]

        raise RuntimeError('foreground sync job failed')

    def get_default_setup(self,force):
        if not force and self.opts.noauto:
            return ''

        return agentd.get_default_setup()

    def initialise(self,frontend,scaffold,cookie,info):
        self.scaffold = scaffold
        self.frontend = frontend
        self.frontend_context = piw.tsd_snapshot()

        self.garbage_context = self.__create_context('eigend-garbage',None,False)

        piw.tsd_thing(self.fgthing)

        self.stdio = (sys.stdout,sys.stderr)

        if not self.opts.noredirect:
            sys.stdout = session.Logger()
            sys.stderr = sys.stdout

        self.backend_context = self.__create_context('eigend-backend',None,False)

        def bginit():
            self.agent = Agent(self)
            piw.tsd_thing(self.bgthing)

        self.collector = GarbageCollector(self.scaffold,self.garbage_context)
        self.collector.start()
        self.bugfiler.start(cookie,info)
        self.run_background(bginit)

    def upgrade_setups(self):
        agentd.upgrade_old_setups()

    def prepare_quit(self):
        self.savcond.acquire()
        try:
            while self.saving:
                self.savcond.wait(1)
            self.quitting = True
        finally:
            self.savcond.release()

    def quit(self):
        self.savcond.acquire()
        try:
            if not self.quitting:
                self.prepare_quit()
        finally:
            self.savcond.release()

        if self.agent:
            self.agent.quit()

    def get_logfile(self):
        return resource.get_logfile('eigend') if not self.opts.stdout else ''

    def mediator(self):
        return self.token()

    def get_setups(self):
        return agentd.find_all_setups()

    def get_user_setups(self):
        return agentd.find_user_setups_flat()

    def load_setup(self,setup,upg):
        self.run_background_async(self.agent.load_file,setup,upg)

    def __alert_dialog(self,klass,label,text):
        self.frontend.alert_dialog(klass,label,text)

    def alert_dialog(self,klass,label,text):
        self.run_foreground_async(self.__alert_dialog,klass,label,text)

    def __info_dialog(self,klass,label,text):
        self.frontend.info_dialog(klass,label,text)

    def info_dialog(self,klass,label,text):
        self.run_foreground_async(self.__info_dialog,klass,label,text)

    def __setups_changed(self,file):
        self.frontend.setups_changed(file)

    def __set_default_setup(self,path):
        agentd.set_default_setup(path)

    def setups_changed(self,file=None):
        self.run_foreground_async(self.__setups_changed,file or '')

    def set_default_setup(self,path):
        self.run_foreground_async(self.__set_default_setup,path)

    def delete_setup(self,slot):
        agentd.delete_user_slot(slot)
        self.setups_changed()

    def save_setup(self,slot,tag,desc,make_default):
        self.savcond.acquire()
        try:
            if self.quitting:
                return
            else:
                self.saving = True
        finally:
            self.savcond.release()

        filename = agentd.user_setup_file(slot,tag)
        filenamesave = filename+"~"

        def done(*args,**kwds):
            agentd.delete_user_slot(slot)
            os.rename(filenamesave,filename)

            if make_default:
                agentd.set_default_setup(filename)

            self.setups_changed(filename)

            self.savcond.acquire()
            try:
                self.saving = False
                self.savcond.notify()
            finally:
                self.savcond.release()

            print 'save complete',filename
            self.info_dialog('Setup Saved','Setup Saved',"The user setup '"+slot+"' was successfully saved")

        r = self.run_background(self.agent.save_file,filenamesave,desc)
        r.setCallback(done,r).setErrback(done,r)
        return filename

    def edit_setup(self,orig,slot,tag,desc):
        self.savcond.acquire()
        try:
            if self.quitting:
                return
            else:
                self.saving = True
        finally:
            self.savcond.release()

        filename = agentd.user_setup_file(slot,tag)
        self.agent.edit_file(orig,filename,desc)

        self.setups_changed(filename)

        self.savcond.acquire()
        try:
            self.saving = False
            self.savcond.notify()
        finally:
            self.savcond.release()

        print 'editing complete',filename
        self.info_dialog('Setup Edited','Setup Edited',"The user setup '"+slot+"' was successfully edited")

        return filename

    def get_description(self,setup):
        return upgrade.get_description_from_file(setup)

    def __get_email(self):
        filename = resource.user_resource_file('global',resource.user_details,version='')
        try:
            l = open(filename,'r').readlines()
            return l[0].strip(),l[1].strip()
        except:
            return ('','')

    def get_email(self):
        return self.__get_email()[1]

    def get_username(self):
        return self.__get_email()[0]

    def get_subject(self):
        return 'Bug Report: ' + time.strftime("%a, %d %b %Y")

    def file_bug(self,user,email,subj,desc):
        filename = resource.user_resource_file('global',resource.user_details,version='')
        open(filename,'w').write("%s\n%s\n" % (user.strip(),email.strip()))
        self.bugfiler.file(user,email,subj,desc)

    def get_setup_slot(self,slot):
        return agentd.get_setup_slot(slot)

    def notes_to_words(self,notes):
        return translate.notes_to_words(notes)

    def words_to_notes(self,words):
        return translate.words_to_notes(words)

def main():
    return Backend()
