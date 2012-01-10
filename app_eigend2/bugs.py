
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

from pi import resource,utils
from app_eigend2 import version
import piw

import zipfile
import glob
import os.path
import time
import sys
import base64
import httplib
import mimetypes
import os
import threading
import xmlrpclib
import datetime

class HttpError(RuntimeError):
    pass

def get_latest_release(cookie,additional=""):
    try:
        srv = xmlrpclib.Server('https://www.eigenlabs.com/xmlrpc/')
        (myversion,mytag) = resource.split_version(version.version)
        if not mytag: mytag='stable'
        #latest = srv.get_latest_tag(mytag)
        latest = srv.get_latest_tag_extra(mytag,cookie,version.version,additional)
        latestv = resource.split_version(latest)[0]
        if latestv > myversion:
            return latest
    except:
        pass

    return ''

def file_bug(user,email,subj,desc):
    zf = resource.get_bugfile()
    zip = zipfile.ZipFile(zf, 'w')

    zip.writestr('description',desc)
    zip.writestr('user',user)
    zip.writestr('email',email)
    zip.writestr('subject',subj)

    zip.writestr('bugreport.txt', "From: %s <%s>\n\nSubject: %s\n\n%s\n" % (user,email,subj,desc))

    def_state_file = resource.user_resource_file('global',resource.current_setup)
    for statefile in glob.glob(def_state_file+'*'):
        zip.write(statefile,'setup/'+os.path.basename(statefile),compress_type=zipfile.ZIP_DEFLATED)

    # add the crash reports of today to the bug report
    if resource.is_macosx():
        diag = os.path.expanduser("~/Library/Logs/DiagnosticReports")
        today = datetime.date.today().strftime("_%Y-%m-%d-")
        if os.path.isdir(diag):
            for crashfile in glob.glob( "%s/eigen*%s*.crash" % (diag, today) ):
                zip.write(crashfile,"crash/"+os.path.basename(crashfile),compress_type=zipfile.ZIP_DEFLATED)
            for crashfile in glob.glob( "%s/Workbench*%s*.crash" % (diag, today) ):
                zip.write(crashfile,"crash/"+os.path.basename(crashfile),compress_type=zipfile.ZIP_DEFLATED)
    
    #core_files = glob.glob('/cores/*')
    #if core_files:
    #    zip.write( core_files[0],compress_type=zipfile.ZIP_DEFLATED)

    log_folder = resource.user_resource_dir( 'log' )
    for file in glob.glob( log_folder + "/*" ):
        path,filename = os.path.split( file )
        full_path = os.path.join( 'log',filename)
        zip.write( file, full_path,compress_type=zipfile.ZIP_DEFLATED )    
    
    zip.close()


def post_multipart(host, selector, fields, files):
    """
    Post fields and files to an http host as multipart/form-data.
    fields is a sequence of (name, value) elements for regular form fields.
    files is a sequence of (name, filename, value) elements for data to be uploaded as files
    Return the server's response page.
    """
    content_type, body = encode_multipart_formdata(fields, files)
    h = httplib.HTTP(host)
    h.putrequest('POST', selector)
    h.putheader('content-type', content_type)
    h.putheader('content-length', str(len(body)))
    h.endheaders()
    h.send(body)
    errcode, errmsg, headers = h.getreply()
    if errcode!=200:
        raise HttpError('%s (%d)' % (errmsg,errcode))

def encode_multipart_formdata(fields, files):
    """
    fields is a sequence of (name, value) elements for regular form fields.
    files is a sequence of (name, filename, value) elements for data to be uploaded as files
    Return (content_type, body) ready for httplib.HTTP instance
    """
    BOUNDARY = '----------ThIs_Is_tHe_bouNdaRY_$'
    CRLF = '\r\n'
    L = []
    for (key, value) in fields:
        L.append('--' + BOUNDARY)
        L.append('Content-Disposition: form-data; name="%s"' % key)
        L.append('')
        L.append(value)
    for (key, filename, value) in files:
        L.append('--' + BOUNDARY)
        L.append('Content-Disposition: form-data; name="%s"; filename="%s"' % (key, filename))
        L.append('Content-Type: %s' % get_content_type(filename))
        L.append('Content-Transfer-Encoding: base64')
        L.append('')
        L.append(base64.encodestring(value))
    L.append('--' + BOUNDARY + '--')
    L.append('')
    body = CRLF.join(L)
    content_type = 'multipart/form-data; boundary=%s' % BOUNDARY
    return content_type, body

def get_content_type(filename):
    return mimetypes.guess_type(filename)[0] or 'application/octet-stream'

def get_bug():
    dir = resource.user_resource_dir('bugs',version='')
    bugs = glob.glob(os.path.join(dir,'*.zip'))
    return bugs[0] if bugs else None

def send_bug(bug):
    zip = zipfile.ZipFile(bug, 'r')
    subject = zip.read('subject')
    user = zip.read('user')
    email = zip.read('email')
    description = zip.read('description')

    fields = (('subject',subject),('from',email),('description',description))
    files = (('report','report.zip',open(bug,'rb').read()),)

    post_multipart('www.eigenlabs.com','/bugfiler/', fields, files)


def send_one_bug():
    bug = get_bug()

    if not bug:
        return 0

    try:
        print 'sending',bug
        send_bug(bug)
        os.unlink(bug)
        print 'sent',bug
        return 1
    except:
        print 'failed to send',bug
        #utils.log_exception()
        return -1

class BugFiler(threading.Thread):
    def __init__(self,backend):
        threading.Thread.__init__(self)
        self.__event = threading.Event()
        self.__context = piw.tsd_snapshot()
        self.__backend = backend
        self.__cookie = ""

    def file(self,user,email,subj,desc):
        file_bug(user,email,subj,desc)
        self.__event.set()

    def start(self,cookie,info):
        self.__cookie = cookie
        self.__info = info
        threading.Thread.start(self)

    def run(self):
        self.__context.install()
        while True:
            print 'bugfiler running',self.__cookie
            self.__backend.set_latest_release(get_latest_release(self.__cookie,self.__info))
            while send_one_bug()>0:
                continue
            print 'bugfiler sleeping'
            self.__event.wait(3600)
            self.__event.clear()

        print 'bugfiler exiting'
