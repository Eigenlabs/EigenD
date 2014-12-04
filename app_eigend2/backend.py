
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

from pi import agent,resource,utils,state
from pibelcanto import translate
from app_eigend2 import version
from pisession import agentd,session,upgrade

import bugs_cli
import latest_release

import piw
import picross
import os
import sys
import time
import threading
import traceback
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



class Backend(eigend_native.c2p):
    def __init__(self):
        eigend_native.c2p.__init__(self)
        self.latest_release = latest_release.LatestReleasePoller(self)
        self.collector = None
        self.__progress = None
        self.__progress_lock = threading.Lock()
        self.saving = False
        self.quitting = False
        self.savcond = threading.Condition()
        self.current_setup = None
        self.current_setup_user = False

        self.fgthing = piw.thing()
        self.fgthing.set_slow_trigger_handler(utils.notify(self.fgdequeue))
        self.fgqueue = []

        self.bgthing = piw.thing()
        self.bgthing.set_slow_trigger_handler(utils.notify(self.bgdequeue))
        self.bgqueue = []
        self.frontend = None

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
        if self.frontend:
            self.frontend.load_started(setup)

    def __load_ended(self,errors):
        if self.frontend:
            self.frontend.load_ended()
            if errors:
                self.frontend.alert_dialog('Load Problems','Load Problems','\n'.join(errors))

    def __set_latest_release(self,release):
        if self.frontend:
            self.frontend.set_latest_release(release)

    def load_started(self,setup):
        self.run_foreground_async(self.__load_started,setup)

    def load_ended(self,errors=[]):
        self.run_foreground_async(self.__load_ended,errors)

    def __load_status(self):
        old = None

        self.__progress_lock.acquire()
        try:
            old = self.__progress
            self.__progress = None
        finally:
            self.__progress_lock.release()

        if old and self.frontend:
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

    def get_default_setup(self,init):
        if init and self.opts.noauto:
            return ''

        setup = agentd.get_default_setup()
        if not init and setup is None:
            return ''

        if setup is None:
            return agentd.get_detected_setup()

        return setup

    def __create_context(self,name):
        if self.frontend:
            logger = self.frontend.make_logger(name)
            return self.scaffold.bgcontext(piw.tsd_scope(),utils.statusify(None),logger,name)
        else:
            return None

    @utils.nothrow
    def initialise(self,frontend,scaffold,cookie,info):
        try:
            self.scaffold = scaffold
            self.frontend = frontend
            self.frontend_context = piw.tsd_snapshot()

            self.garbage_context = self.__create_context('eigend-garbage')

            piw.tsd_thing(self.fgthing)

            self.stdio = (sys.stdout,sys.stderr)

            if not self.opts.noredirect:
                sys.stdout = session.Logger()
                sys.stderr = sys.stdout

            self.backend_context = self.__create_context('eigend-backend')

            resource.clean_current_setup()

            def bginit():
                self.agent = agentd.Agent(self,1)
                piw.tsd_thing(self.bgthing)

            self.collector = GarbageCollector(self.scaffold,self.garbage_context)
            self.collector.start()
			
            if os.getenv('PI_NOCHECK') is None:
                self.latest_release.start(cookie,info)

            self.run_background(bginit)
        except:
            print >>sys.__stdout__,'Initialisation failure'
            traceback.print_exc(limit=None,file=sys.__stdout__)
            raise

    def upgrade_setups(self):
        agentd.upgrade_default_setup()
        agentd.upgrade_old_setups()

    def prepare_quit(self):
        self.savcond.acquire()
        try:
            if self.saving:
                return False
            self.quitting = True
            self.frontend = None
            return True
        finally:
            self.savcond.release()

    def quit(self):
        self.savcond.acquire()
        try:
            self.quitting = True
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

    def load_setup(self,setup,user):
        self.set_current_setup(setup,user)
        self.run_background_async(self.agent.load_file,setup)

    def __alert_dialog(self,klass,label,text):
        if self.frontend:
            self.frontend.alert_dialog(klass,label,text)

    def alert_dialog(self,klass,label,text):
        self.run_foreground_async(self.__alert_dialog,klass,label,text)

    def __info_dialog(self,klass,label,text):
        if self.frontend:
            self.frontend.info_dialog(klass,label,text)

    def info_dialog(self,klass,label,text):
        self.run_foreground_async(self.__info_dialog,klass,label,text)

    def __setups_changed(self,file):
        if self.frontend:
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

    def save_rename(self,root,prefix,final):
        root_d = os.path.dirname(root)
        root_f = prefix+os.path.basename(root)
        prefix_len = len(prefix)

        for f in resource.os_listdir(root_d):
            if f.startswith(root_f):
                f2 = os.path.join(root_d,f[prefix_len:])
                try: resource.os_unlink(f2)
                except: pass
                resource.os_rename(os.path.join(root_d,f),f2)

    def save_tmp(self,root,prefix):
        root_d = os.path.dirname(root)
        root_f = prefix+os.path.basename(root)
        return os.path.join(root_d,root_f)

    def set_current_setup(self,setup,user):
        self.current_setup = setup
        self.current_setup_user = user

    def save_current_setup(self):
        if not self.current_setup or not self.current_setup_user:
            return False
        
        term = self.get_user_setups()
        for i in range(1,term.arity()):
            if self.current_setup == term.arg(i).arg(2).value().as_string():
                slot = term.arg(i).arg(1).value().as_string()
                tag = None
                if term.arg(i).arg(0).value().is_string():
                    tag = term.arg(i).arg(0).value().as_string()
                desc = self.get_description(self.current_setup)
                self.save_setup(slot,tag,desc,False)
                return True

        return False

    def save_setup(self,slot,tag,desc,make_default):
        print 'save setup',slot,tag,desc,make_default
        self.savcond.acquire()
        try:
            if self.quitting:
                return
            else:
                self.saving = True
        finally:
            self.savcond.release()

        filename = agentd.user_setup_file(slot,tag)

        def not_done(*args,**kwds):
            print 'save failed:',args
            self.info_dialog('Setup Not Saved','Setup Not Saved',"The user setup '"+slot+"' could not be saved\n"+args[1])

        def done(*args,**kwds):
            agentd.delete_user_slot(slot)
            self.save_rename(filename,'~',filename)

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

        r = self.run_background(self.agent.save_file,self.save_tmp(filename,'~'),desc)
        r.setCallback(done,r).setErrback(not_done,r)
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

        path = agentd.user_setup_file(slot,tag)

        if orig!=path:
            orig_d = os.path.dirname(orig)
            orig_f = os.path.basename(orig)
            path_d = os.path.dirname(path)
            path_f = os.path.basename(path)

            for f in resource.os_listdir(orig_d):
                if f.startswith(orig_f):
                    f2 = os.path.join(path_d,path_f+f[len(orig_f):])
                    resource.os_rename(os.path.join(orig_d,f),f2)

        database = state.open_database(path,True)
        trunk = database.get_trunk()
        upgrade.set_description(trunk,desc)
        trunk.save(piw.tsd_time(),'')
        database.flush()

        self.setups_changed(path)

        self.savcond.acquire()
        try:
            self.saving = False
            self.savcond.notify()
        finally:
            self.savcond.release()

        print 'editing complete',path
        self.info_dialog('Setup Edited','Setup Edited',"The user setup '"+slot+"' was successfully edited")

        return path

    def get_description(self,setup):
        return upgrade.get_description_from_file(setup)

    def __get_email(self):
        filename = resource.user_resource_file(resource.global_dir,resource.user_details,version='')
        try:
            l = resource.file_open(filename,'r').readlines()
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
        filename = resource.user_resource_file(resource.global_dir,resource.user_details,version='')
        resource.file_open(filename,'w').write("%s\n%s\n" % (user.strip(),email.strip()))
        bugs_cli.file_bug(user,email,subj,desc)

    def get_setup_slot(self,slot):
        return agentd.get_setup_slot(slot)

    def notes_to_words(self,notes):
        return translate.notes_to_words(notes)

    def words_to_notes(self,words):
        return translate.words_to_notes(words)

def main():
    return Backend()
