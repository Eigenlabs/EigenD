
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

# ------------------------------------------------------------------------------------------------------------------
# Convolver plugin
#
# Convolves a browseable impulse response with incoming audio for producing reverb, delay effects, instrument bodies etc.
# ------------------------------------------------------------------------------------------------------------------

# TODO:
# 1. allow slow thread inputs to save their state

import os
import glob
import sys
import piw
import picross
import math
from pi import  action,agent,atom,bundles,domain,files,paths,policy,resource,riff,upgrade,utils,node,logic,index,guid,rpc,async,state,container
from pi.logic.shortcuts import T
from . import convolver_version as version, convolver_native

def volume_function(db):
    if db==-24: return 0.0
    sc = pow(10.0,db/20.0)
    return sc

# ------------------------------------------------------------------------------------------------------------------
# WavSample class: reads a wav (riff) file (for impulse response)
#
# Taken from clicker_plg.py, factor these?
# ------------------------------------------------------------------------------------------------------------------

class WavSample:
    def read(self,c):
        return c.read_external(self.__external)

    def __external(self,file,len):
        return file.read(len)

wav_reader = riff.Root('WAVE', riff.List(**{ 'fmt ': riff.Struct('<hHLLHH'), 'data': WavSample() }))

def fgetsamples(filename):
    f = resource.file_open(filename,'rb',0)
    r = wav_reader.read(f)
    chans = r['fmt '][1]
    rate = r['fmt '][2]
    bps = r['fmt '][5]
    return convolver_native.canonicalise_samples(r['data'],rate,chans,bps)

def rgetsamples(res):
    from cStringIO import StringIO
    r = files.PkgResourceFile(res)
    r2 = wav_reader.read(StringIO(r.data(0,r.size())))
    chans = r2['fmt '][1]
    rate = r2['fmt '][2]

    bps = r2['fmt '][5]
    return convolver_native.canonicalise_samples(r2['data'],rate,chans,bps)

def wav_resource(name):
    # this is the impulse response directory
    uf = resource.user_resource_file(resource.impulseresponse_dir,name,version='')
    if resource.os_path_isfile(uf):
        return fgetsamples(uf)
    return rgetsamples('plg_convolver/%s'%name)


# ------------------------------------------------------------------------------------------------------------------
# Impulse response browser agent
#
#
# ------------------------------------------------------------------------------------------------------------------

class ImpulseBrowser(atom.Atom):
    userdir = os.path.join(resource.user_resource_dir('ImpulseResponse',version=''))
    reldir = os.path.join(picross.global_resource_dir(),'impulseresponse')

    def __init__(self,agent):
        self.__scan()
        self.__selected=None
        self.agent = agent
        atom.Atom.__init__(self,names='impulse',protocols='virtual browse',domain=domain.String(),policy=atom.default_policy(self.__setcurrent))
        self.__timestamp = piw.tsd_time()
        self.__update()

    # ---------------------------------------------------------------------------------------------
    # functions to support selection of impulse by browsing
    # ---------------------------------------------------------------------------------------------

    # update: update the time stamp serial number so that browser can know
    # if the browsing state has been updated
    def __update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def rpc_displayname(self,arg):
        return 'convolver impulses'
  
    # setselected: select (highlight) the item in the selection list
    def rpc_setselected(self,arg):
        (path,selected) = logic.parse_clause(arg)
        # __selected is the cookie (unique key), cookie=filename stripped of extension
        self.__selected = selected
 
    # activated: activate choosing the item in the selection list
    def rpc_activated(self,arg):
        (path,selected) = logic.parse_clause(arg)
        # set the current to be the cookie
        self.__setcurrent(selected)
        return logic.render_term(('',''))
   
    # setcurrent: set the chosen item in the selection list
    def __setcurrent(self,cookie):
        if cookie!=self.get_value():
            # cookie is the stripped filename
            if not self.__f2p.has_key(cookie):
                return False
            filepath = self.__f2p[cookie]
            self.agent.load_impulse_response(filepath)
            self.set_value(cookie)
            self.__update()
        return True

    # current: return the current selection
    def rpc_current(self,arg):
        c = self.get_value()
        # nothing selected yet
        if not c:
            return '[["",[]]]'
        # return cookie, no category
        return '[["%s",[]]]' % c

    # enumerate: get the lengths of the selection and subcategory lists
    def rpc_enumerate(self,a):
        path = logic.parse_clause(a)
        self.__scan()

        # no subcategory, so just return the length of the selection list
        return logic.render_term((len(self.__f2p), 0))

    # cinfo: get the subcategory list
    def rpc_cinfo(self,a):
        # no category
        return '[]'

    # finfo: get the selection list
    def rpc_finfo(self,a):
        (path,idx) = logic.parse_clause(a)
        self.__scan()

        if len(path) == 0:
            # list of tuples for the browser window, [(cookie,textcolumn1,textcolumn2),...]
            map = tuple([(files,files,self.__c2n.get(files)) for files in self.__files[idx:]])
            mapstr = logic.render_term(map)
            return mapstr

        return logic.render_term(())
       
    # scan1: scan release and user directories for wav files
    def __scan1(self,pat,f=lambda x:x):
        g = lambda path: resource.glob_glob(os.path.join(path,pat))
        paths = g(self.reldir) + g(self.userdir)
        b = lambda path: os.path.splitext(os.path.basename(path))[0]
        # return filenames without extensions, 
        # and function applied to paths
        return map(b,paths), map(f,paths)

    # scan: scan for wav files and create a list of filenames
    def __scan(self):
        # look for files ended in .wav
        files,paths = self.__scan1('*.[wW][aA][vV]')
        # dict of filenames to their full paths
        self.__f2p = dict(zip(files,paths))
        # list of filenames
        self.__files = self.__f2p.keys()
        # read .name files whose filenames are belcanto names (phrases) and contents are the 
        # impulse cookies that the belcanto maps to
        names,cookies = self.__scan1('*.name', lambda p: resource.file_open(p).read().strip())
        # remove name from filenames
        names = [name.replace('_',' ') for name in names]
        self.__n2c = dict(zip(names,cookies))
        self.__c2n = dict(zip(cookies,names))


    # ---------------------------------------------------------------------------------------------
    # functions to support selection of impulse by belcanto
    # ---------------------------------------------------------------------------------------------

    # choose_cookie: select from agent choose verb
    def choose_cookie(self,c):
        if self.__setcurrent(c):
            return action.nosync_return()
        return async.failure('choose_cookie failed %s' % c) 

    # called from resolve
    def __ideals(self,*cookies):
        return '[%s]' % ','.join([self.__ideal(c) for c in cookies])

    # ideal: return prolog of cookie 
    # called from fideal
    def __ideal(self,cookie):
        return 'ideal([~server,impulse],"%s")' % cookie

    # fideal: 
    # called from the ideal rpc on the agent with type impulse
    def rpc_fideal(self,arg):
        (path,cookie) = logic.parse_clause(arg)
        return self.__ideal(cookie)

    # resolve: resolve belcanto into 
    def rpc_resolve(self,arg):
        (a,o) = logic.parse_clause(arg)
        if a == ('current',) and o is None:
            c = self.get_value()
            if c:
                return self.__ideals(self.get_value())
        return self.__ideals()

    # resolve_name:  
    # called by agent resolve_ideal rpc
    def resolve_name(self,n):
        if n=='selection':
            if self.__selected:
                return self.__ideals(self.__selected)
        
        # name filenames have _ separators
        #n = n.replace(' ','_')
        self.__scan()
        c = self.__n2c.get(n)
        if c:
            return self.__ideals(c)
        return self.__ideals()




# ------------------------------------------------------------------------------------------------------------------
# Convolver agent
#
#
# ------------------------------------------------------------------------------------------------------------------

class Agent(agent.Agent):

    def __init__(self,address, ordinal):
        # the agent event clock
        self.domain = piw.clockdomain_ctl()

        # verb container, used by all taps
        agent.Agent.__init__(self,signature=version,names='convolver',container=3,ordinal=ordinal)

        self.vol = piw.make_f2f_table(-24,24,1000,picross.make_f2f_functor(volume_function))

        # outputs 
        self[1]=bundles.Output(1,True,names="left audio output")
        self[2]=bundles.Output(2,True,names="right audio output")
        self.output = bundles.Splitter(self.domain, self[1], self[2])

        # the convolver class
        self.convolver = convolver_native.convolver(self.vol,self.output.cookie(),self.domain)

        # input has the correlator and a bundle style output
        self.input = bundles.ScalarInput(self.convolver.cookie(), self.domain, signals=(1,2,3,4))

        # self[3] = verb container

        self[4] = ImpulseBrowser(self)

        # audio inputs
        # use vector policy inputs
        self[5]=atom.Atom(domain=domain.BoundedFloat(-1,1), names="left audio input", policy=self.input.nodefault_policy(1,True))
        self[6]=atom.Atom(domain=domain.BoundedFloat(-1,1), names="right audio input", policy=self.input.nodefault_policy(2,True))

        # wet/dry mix
        self[7]=atom.Atom(domain=domain.BoundedFloat(-24,24,hints=(T('stageinc',0.1),T('inc',0.1),T('biginc',1),T('control','updown'))), init=0, names="dry gain", protocols='input', policy=self.input.merge_policy(3,False))
        self[11]=atom.Atom(domain=domain.BoundedFloat(-24,24,hints=(T('stageinc',0.1),T('inc',0.1),T('biginc',1),T('control','updown'))), init=0, names="wet gain", protocols='input', policy=self.input.merge_policy(4,False))
        # effect enable
        self[8]=atom.Atom(domain=domain.Bool(), init=True, names="enable", protocols='input', policy=atom.default_policy(self.__set_enable))
        # mono processing mode
        self[9]=atom.Atom(domain=domain.Bool(), init=False, names="mono", policy=atom.default_policy(self.__set_mono_processing))
        # enable time, time to fade in and out when enabling in ms
        self[10]=atom.Atom(names='enable time input', domain=domain.BoundedFloat(0,100000,hints=(T('stageinc',1),T('inc',1),T('biginc',10),T('control','updown'))), init=100, policy=atom.default_policy(self.__set_enable_time))

        self.__set_enable(self[8].get_value())
        self.__set_mono_processing(self[9].get_value())
        self.__set_enable_time(self[10].get_value())

        self.add_verb2(1,'choose([],None,role(None,[ideal([~server,impulse]),singular]))',self.__choose)

    # resolve: resolve an impulse belcanto name into an impulse cookie
    def rpc_resolve_ideal(self,arg):
        (type,arg) = action.unmarshal(arg)
        # only support 'impulse' type
        if type=='impulse':
            return self[4].resolve_name(' '.join(arg))
        return action.marshal(())

    # choose: implement choosing an impulse response with the choose verb
    def __choose(self,subj,arg):
        # resolve the ideal, e.g. type is 'impulse', and cookie is the impulse cookie 
        (type,cookie) = action.crack_ideal(action.arg_objects(arg)[0])
        return self[4].choose_cookie(cookie)

    # load_impulse_response: load impulse response bytes into convolution engine
    def load_impulse_response(self,filename):
        # load the wav file
        impulse_response = wav_resource(filename)
        # pass wav data to convolution agent
        self.convolver.set_impulse_response(impulse_response)

    # set mono processing mode
    def __set_mono_processing(self,b):
        self.convolver.set_mono_processing(b)
        return True

    # set enable
    def __set_enable(self,e):
        self.convolver.set_enable(e)
        return True
     
    # set enable time
    def __set_enable_time(self,t):
        self.convolver.set_enable_time(t)
        return True

class Upgrader(upgrade.Upgrader):
    def upgrade_1_0_0_to_1_0_1(self,tools,address):
        print 'upgrading convolver',address
        root = tools.get_root(address)
        dry_vol = root.get_node(7,254).get_data().as_float()
        wet_vol = 1-dry_vol
        dry_db = 20*math.log10(dry_vol)
        wet_db = 20*math.log10(wet_vol)+24 # we're now automatically reducing by 24 db in the impulse importer
        print 'dry vol',dry_vol,'dry db',dry_db,'wet vol',wet_vol,'wet db',wet_db
        root.get_node(7,254).set_data(piw.makefloat_bounded(24,-24,0,dry_db,0))
        root.ensure_node(11,254).set_data(piw.makefloat_bounded(24,-24,0,wet_db,0))

agent.main(Agent,Upgrader)

# ------------------------------------------------------------------------------------------------------------------

