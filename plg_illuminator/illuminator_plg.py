#
# Copyright 2012 Eigenlabs Ltd.  http://www.eigenlabs.com
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

from pi import agent,async,atom,collection,const,domain,policy,bundles,logic,action,utils,resource
from . import illuminator_version as version

import piw
import threading,sys,socket,fileinput,re
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer

MATCH_PHYSICAL = re.compile('^/column/(\d+)/row/(\d+)$',re.IGNORECASE)
MATCH_MUSICAL = re.compile('^/course/(\d+)/key/(\d+)$',re.IGNORECASE)
MATCH_MAP = re.compile('^\[(?:\[\[\d+,\d+\],\w+\](?:,\[\[\d+,\d+\],\w+\])*)?\]$',re.IGNORECASE)
MATCH_BITMAP = re.compile('^[RGO \\n\\,\\.]*$',re.IGNORECASE|re.MULTILINE)

class InterruptableHTTPServer(HTTPServer):
    allow_reuse_address = True

    def __init__(self, agent, snapshot, host, port, handler):
        HTTPServer.__init__(self, (host,port), handler)
        self.agent = agent
        self.__snapshot = snapshot

    def server_bind(self):
        self.__timeToQuit = threading.Event()
        self.__timeToQuit.clear()      

        HTTPServer.server_bind(self)
        self.socket.settimeout(1)

    def get_request(self):
        while not self.__timeToQuit.isSet():
            try:
                sock, addr = self.socket.accept()
                sock.settimeout(None)
                return (sock, addr)
            except socket.timeout:
                if self.__timeToQuit.isSet():
                    raise socket.error
        raise socket.error

    def close_request(self, request):
        if (request is None): return
        HTTPServer.close_request(self, request)

    def process_request(self, request, client_address):
        if (request is None): return
        HTTPServer.process_request(self, request, client_address)

    def start(self):
        self.__timeToQuit.clear()      
        threading.Thread(target=self.serve).start()

    def stop(self):
        self.__timeToQuit.set()

    def serve(self):
        self.__snapshot.install()

        while not self.__timeToQuit.isSet():
            try:
                self.handle_request()
            except:
                print 'server error: unexpected error', sys.exc_info()[0]
        self.server_close()

class IlluminatorRequestHandler(BaseHTTPRequestHandler):
    def output_get_plain(self, content):
        self.send_response(200)
        self.send_header("Content-Type", "text/plain")                     
        self.end_headers()
        self.wfile.write(content)

    def output_get_xhtml(self, content):
        self.send_response(200)
        self.send_header("Content-Type", "application/xhtml+xml")                     
        self.end_headers()
        self.wfile.write(content)

    def parse_bitmap(self, bitmap):
        result = []
        column = 1 
        row = 0
        for c in bitmap.lower():
            row += 1
            if c == 'r':
                result.append([[column,row],'red'])
            elif c == 'o':
                result.append([[column,row],'orange'])
            elif c == 'g':
                result.append([[column,row],'green'])
            elif c == '\n' or c == ',' or c == '.':
                column += 1
                row = 0
        return result
    
    def send_success(self):
        self.send_response(200)
        self.send_header("Content-Type", "text/plain")                     
        self.end_headers()

    def do_OPTIONS(self):
        self.send_success()
        return

    def do_GET(self):
        if self.path == '/':
            xhtml = open(resource.find_release_resource('illuminator','illuminator_plg.xhtml'))
            self.output_get_xhtml(xhtml.read())
            xhtml.close()
            return

        if self.path == '/physical':
            self.output_get_plain(self.server.agent.get_physical())
            return

        if self.path == '/musical':
            self.output_get_plain(self.server.agent.get_musical())
            return

        self.send_response(404)
        self.end_headers()

    def do_PUT(self):
        if self.path == '/physical':
            content = self.rfile.read(int(self.headers['Content-Length']))
            if MATCH_MAP.match(content):
                self.send_success()
                self.server.agent.set_physical_map(content)
            elif MATCH_BITMAP.match(content):
                self.send_success()
                self.server.agent.set_physical_map(logic.render_term(self.parse_bitmap(content)))
            else:
                self.send_response(400)
                self.end_headers()
            return

        if self.path == '/musical':
            content = self.rfile.read(int(self.headers['Content-Length']))
            if MATCH_MAP.match(content):
                self.send_success()
                self.server.agent.set_musical_map(content)
            elif MATCH_BITMAP.match(content):
                self.send_success()
                self.server.agent.set_musical_map(logic.render_term(self.parse_bitmap(content)))
            else:
                self.send_response(400)
                self.end_headers()
            return

        match = MATCH_PHYSICAL.match(self.path)
        if match:
            self.send_success()
            column = int(match.group(1))
            row = int(match.group(2))
            colour = self.rfile.read(int(self.headers['Content-Length']))
            self.server.agent.set_physical(column,row,colour)
            return

        match = MATCH_MUSICAL.match(self.path)
        if match:
            self.send_success()
            course = int(match.group(1))
            key = int(match.group(2))
            colour = self.rfile.read(int(self.headers['Content-Length']))
            self.server.agent.set_musical(course,key,colour)
            return

        self.send_response(404)
        self.end_headers()

    def do_DELETE(self):
        if self.path == '/':
            self.send_success()
            self.server.agent.clear()
            return

        if self.path == '/physical':
            self.send_success()
            self.server.agent.clear_physical()
            return

        if self.path == '/musical':
            self.send_success()
            self.server.agent.clear_musical()
            return

        match = MATCH_PHYSICAL.match(self.path)
        if match:
            self.send_success()
            column = int(match.group(1))
            row = int(match.group(2))
            self.server.agent.unset_physical(column,row)
            return

        match = MATCH_MUSICAL.match(self.path)
        if match:
            self.send_success()
            course = int(match.group(1))
            key = int(match.group(2))
            self.server.agent.unset_musical(course,key)
            return

        self.send_response(404)
        self.end_headers()

class LightMap(atom.Atom):
    def __init__(self,agent,index):
        atom.Atom.__init__(self,names='light mapping',ordinal=index,protocols='remove')

        self[1] = atom.Atom(domain=domain.String(), init='[]', names='physical light mapping', policy=atom.default_policy(self.__physical_light_map))
        self[2] = atom.Atom(domain=domain.String(), init='[]', names='musical light mapping', policy=atom.default_policy(self.__musical_light_map))

    def __physical_light_map(self,v):
        self[1].set_value(v)

    def __musical_light_map(self,v):
        self[2].set_value(v)

class StoredLightMaps(collection.Collection):
    def __init__(self,agent):
        self.agent = agent
        self.__timestamp = piw.tsd_time()

        collection.Collection.__init__(self,names="stored",creator=self.__create_lightmap,wrecker=self.__wreck_lightmap,inst_creator=self.__create_inst,inst_wrecker=self.__wreck_inst)
        self.update()

    def update(self):
        self.__timestamp = self.__timestamp+1
        self.set_property_string('timestamp',str(self.__timestamp))

    def new_lightmap(self,index):
        return LightMap(self.agent, index)
    
    def lightmaps_changed(self):
        pass
    
    def __create_lightmap(self,index):
        return self.new_lightmap(index)
    
    def __wreck_lightmap(self,index,node):
        self.lightmaps_changed()

    def create_lightmap(self,ordinal=None):
        o = ordinal or self.find_hole()
        o = int(o)
        e = self.new_lightmap(o)
        self[o] = e
        e.set_ordinal(int(o))
        self.lightmaps_changed()
        self.agent.update()
        return e 

    def get_lightmap(self,index):
        return self.get(index)

    def load(self,oid):
        for k,v in self.items():
            if v.id()==oid:
                self.agent.set_physical_map(v[1].get_value())
                self.agent.set_musical_map(v[2].get_value())
                return True

        return False

    def uncreate(self,oid):
        for k,v in self.items():
            if v.id()==oid:
                self.del_lightmap(k)
                return True

        return False

    def del_lightmap(self,index):
        v = self[index]
        del self[index]
        self.lightmaps_changed()
        self.agent.update()
    
    @async.coroutine('internal error')
    def __create_inst(self,ordinal=None):
        e = self.create_lightmap(ordinal)
        yield async.Coroutine.success(e)

    @async.coroutine('internal error')
    def __wreck_inst(self,key,inst,ordinal):
        self.lightmaps_changed()
        yield async.Coroutine.success()

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='illuminator', ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()

        self[7] = bundles.Output(1, False, names='server status output')
        self.server_status_output = bundles.Splitter(self.domain, self[7])
        self.server_lights = piw.lightsource(piw.change_nb(), 0, self.server_status_output.cookie())
        self.server_lights.set_size(1)        

        self[1] = bundles.Output(1, False, names='light output',protocols='revconnect')
        self.output = bundles.Splitter(self.domain, self[1])

        self.status_buffer = piw.statusbuffer(self.output.cookie())
        self.status_buffer.autosend(False)
 
        self[2] = atom.Atom(domain=domain.String(), init='[]', names='physical light mapping', policy=atom.default_policy(self.__physical_light_map))
        self[3] = atom.Atom(domain=domain.String(), init='[]', names='musical light mapping', policy=atom.default_policy(self.__musical_light_map))
        self[5] = atom.Atom(domain=domain.BoundedIntOrNull(0,65535,0), names='server port', policy=atom.default_policy(self.__server_port))
        self[6] = atom.Atom(domain=domain.Bool(), init=False, names='server start', policy=atom.default_policy(self.__server_start))

        self.keyfunctor = piw.functor_backend(1, False)
        self.keyinput = bundles.VectorInput(self.keyfunctor.cookie(), self.domain, signals=(1,))
        self[4] = atom.Atom(domain=domain.Aniso(), policy=self.keyinput.vector_policy(1,False), names='key input')
        self.choicefunctor = utils.make_change_nb(piw.slowchange(utils.changify(self.__choice)))

        self[8] = StoredLightMaps(self)

        self.add_verb2(1,'clear([],None)', callback=self.__clear)
        self.add_verb2(2,'clear([],None,role(None,[matches([physical])]))', callback=self.__clear_physical)
        self.add_verb2(3,'clear([],None,role(None,[matches([musical])]))', callback=self.__clear_musical)
        self.add_verb2(4,'set([],None,role(None,[coord(physical,[column],[row])]),role(to,[abstract,matches([red])]))', callback=self.__set_physical)
        self.add_verb2(5,'set([],None,role(None,[coord(physical,[column],[row])]),role(to,[abstract,matches([green])]))', callback=self.__set_physical)
        self.add_verb2(6,'set([],None,role(None,[coord(physical,[column],[row])]),role(to,[abstract,matches([orange])]))', callback=self.__set_physical)
        self.add_verb2(7,'set([],None,role(None,[coord(musical,[course],[key])]),role(to,[abstract,matches([red])]))', callback=self.__set_musical)
        self.add_verb2(8,'set([],None,role(None,[coord(musical,[course],[key])]),role(to,[abstract,matches([green])]))', callback=self.__set_musical)
        self.add_verb2(9,'set([],None,role(None,[coord(musical,[course],[key])]),role(to,[abstract,matches([orange])]))', callback=self.__set_musical)
        self.add_verb2(10,'set([un],None,role(None,[coord(physical,[column],[row])]))', callback=self.__unset_physical)
        self.add_verb2(11,'set([un],None,role(None,[coord(musical,[course],[key])]))', callback=self.__unset_musical)
        self.add_verb2(12,'choose([],None,role(None,[matches([physical])]),role(as,[abstract,matches([red])]))',self.__choose_physical)
        self.add_verb2(13,'choose([],None,role(None,[matches([physical])]),role(as,[abstract,matches([green])]))',self.__choose_physical)
        self.add_verb2(14,'choose([],None,role(None,[matches([physical])]),role(as,[abstract,matches([orange])]))',self.__choose_physical)
        self.add_verb2(15,'choose([],None,role(None,[matches([musical])]),role(as,[abstract,matches([red])]))',self.__choose_musical)
        self.add_verb2(16,'choose([],None,role(None,[matches([musical])]),role(as,[abstract,matches([green])]))',self.__choose_musical)
        self.add_verb2(17,'choose([],None,role(None,[matches([musical])]),role(as,[abstract,matches([orange])]))',self.__choose_musical)
        self.add_verb2(18,'choose([un],None)',self.__unchoose)
        self.add_verb2(19,'start([],None,role(None,[matches([server])]))',self.__start_server,status_action=self.__status_server)
        self.add_verb2(20,'stop([],None,role(None,[matches([server])]))',self.__stop_server,status_action=self.__status_server)
        self.add_verb2(21,'start([toggle],None,role(None,[matches([server])]))',self.__toggle_server,status_action=self.__status_server)
        self.add_verb2(22,'load([],None,role(None,[concrete,singular,partof(~(a)#8)]))', self.__load_lightmap)
        self.add_verb2(23,'delete([],None,role(None,[concrete,singular,partof(~(a)#8)]))', self.__delete_lightmap)
        self.add_verb2(24,'save([],None,role(None,[abstract]))', self.__save_lightmap)
        self.add_verb2(25,'save([],None)', self.__save_lightmap)

        self.httpServer = None

        self.__update_server_status()

    def stop_http(self):
        if self.httpServer:
            print "shutting down HTTP server"
            self.httpServer.stop()
            self.httpServer = None

    def start_http(self):
        self.stop_http()

        port = self[5].get_value()
        if port:
            print "starting up HTTP server on port", port
            self.snapshot = piw.tsd_snapshot()
            server = InterruptableHTTPServer(self, self.snapshot, '0.0.0.0', port, IlluminatorRequestHandler)
            server.start()
            self.httpServer = server

    def __start_server(self,subj=None,v=None):
        try:
            self.start_http()
            self[6].set_value(True)
        except:
            print 'error starting HTTP server on port '+str(self[5].get_value()), sys.exc_info()[0]
        self.__update_server_status()

    def __stop_server(self,subj=None,v=None):
        try:
            self.stop_http()
            self[6].set_value(False)
        except:
            print 'error stopping HTTP server on port '+str(self[5].get_value()), sys.exc_info()[0]
        self.__update_server_status()

    def __toggle_server(self,subj,v):
        if self[6].get_value():
            self.__stop_server()
        else:
            self.__start_server()

    def __status_server(self,subj,v):
        return 'dsc(~(s)"#7",None)'

    def __load_lightmap(self,subj,v):
        a = action.concrete_object(v)
        if self[8].load(a):
            return async.success(action.concrete_return(a))
        return async.success(errors.doesnt_exist('light mapping','load'))

    def __delete_lightmap(self,subj,v):
        a = action.concrete_object(v)
        if self[8].uncreate(a):
            return async.success(action.removed_return(a))
        return async.success(errors.doesnt_exist('light mapping','delete'))

    def __save_lightmap(self,subj,v=None):
        lm = self[8].create_lightmap()
        if lm:
            if v:
                names = action.abstract_string(v).split()
                if names:
                    try:
                        ordinal = int(names[-1])
                        names = names[:-1]
                    except:
                        ordinal = 0
                if len(names) > 0:
                    lm.set_names(' '.join(names))
                lm.set_ordinal(ordinal)
            lm[1].set_value(self[2].get_value())
            lm[2].set_value(self[3].get_value())
            return async.success(action.concrete_return(lm.id()))
        return async.success(errors.cant_error('light mapping','save'))
    
    def __update_server_status(self):
        self.server_lights.set_status(1, const.status_active if self[6].get_value() else const.status_inactive)

    def close_server(self):
        agent.Agent.close_server(self)
        self.stop_http()
 
    def __physical_light_map(self,v):
        self.set_physical_map(v)

    def __musical_light_map(self,v):
        self.set_musical_map(v)

    def __server_port(self,v):
        self[5].set_value(v)

    def __server_start(self,v):
        if v:
            self.__start_server()
        else:
            self.__stop_server()

    def __clear(self,subj):
        self.clear()

    def clear(self):
        self.clear_physical()
        self.clear_musical()

    def __clear_physical(self,subj,v):
        self.clear_physical()

    def clear_physical(self):
        self.set_physical_map('[]')
        self.__update_lights()

    def __clear_musical(self,subj,v):
        self.clear_musical()

    def clear_musical(self):
        self.set_musical_map('[]')

    def get_physical(self):
        return self[2].get_value()

    def set_physical_map(self,content):
        self[2].set_value(content)
        self.__update_lights()

    def __set_physical(self,subject,key,colour):
        column,row = action.coord_value(key)
        colour = action.abstract_string(colour)
        self.set_physical(column,row,colour)

    def set_physical(self,column,row,colour):
        phys = [x for x in logic.parse_clause(self.get_physical()) if x[0][0] != column or x[0][1] != row]
        phys.append([[column,row],colour])
        self.set_physical_map(logic.render_term(phys))

    def get_musical(self):
        return self[3].get_value()

    def set_musical_map(self,content):
        self[3].set_value(content)
        self.__update_lights()

    def __set_musical(self,subject,key,colour):
        course,key = action.coord_value(key)
        colour = action.abstract_string(colour)
        self.set_musical(course,key,colour)

    def set_musical(self,course,key,colour):
        mus = [x for x in logic.parse_clause(self.get_musical()) if x[0][0] != course or x[0][1] != key]
        mus.append([[course,key],colour])
        self.set_musical_map(logic.render_term(mus))

    def __unset_physical(self,subject,key):
        column,row = action.coord_value(key)
        self.unset_physical(column,row)

    def unset_physical(self,column,row):
        phys = [x for x in logic.parse_clause(self.get_physical()) if x[0][0] != column or x[0][1] != row]
        self.set_physical_map(logic.render_term(phys))

    def __unset_musical(self,subject,key):
        course,key = action.coord_value(key)
        self.unset_musical(course,key)

    def unset_musical(self,course,key):
        mus = [x for x in logic.parse_clause(self.get_musical()) if x[0][0] != course or x[0][1] != key]
        self.set_musical_map(logic.render_term(mus))

    def __update_lights(self):
        self.status_buffer.clear()
        self.__add_lights(False,self.get_physical())
        self.__add_lights(True,self.get_musical())
        self.status_buffer.send()
        return True

    def __colour_to_int(self,v):
        colour = str(v).lower()
        if colour == 'red' or colour == 'r':
            colour = 2 
        elif colour == 'green' or colour == 'g':
            colour = 1
        elif colour == 'orange' or colour == 'o':
            colour = 3 
        elif colour == 'off':
            colour = 0
        colour = int(colour)
        if colour < 0 or colour > 3:
            colour = 0
        return colour

    def __add_lights(self,musical,v):
        mapping = logic.parse_clause(v)
        for m in mapping:
            if 2 == len(m) and 2 == len(m[0]):
                self.status_buffer.set_status(musical,piw.coordinate(int(m[0][0]),int(m[0][1])),self.__colour_to_int(m[1]))

    def __choose_physical(self,subject,key,colour):
        self.__choose_base(False,action.abstract_string(colour))

    def __choose_musical(self,subject,key,colour):
        self.__choose_base(True,action.abstract_string(colour))

    def __choose_base(self,musical,colour):
        self.__choices = []
        self.__choosemusical = musical
        self.__choosecolour = colour

        self.status_buffer.clear()
        self.status_buffer.send()
        self.keyfunctor.set_gfunctor(self.choicefunctor)

    def __choice(self,v):
        choice = utils.key_to_lists(v)
        if not choice: return
        if not choice[2]: return
        # remove the hardness so that it's not part of the identity of the choice
        del choice[2]

        # if this choice is the same as the previous one
        # stop choose mode and store the new mapping
        if self.__choices and choice==self.__choices[-1]:
            self.__stop_choosing()
            self.__add_choices()
            return

        # if this is a new choice, store it and adapt the status leds
        if not choice in self.__choices:
            self.__choices.append(choice)
            if self.__choosemusical:
                key = choice[1]
            else:
                key = choice[0]
            self.status_buffer.set_status(self.__choosemusical,piw.coordinate(key[0],key[1]),self.__colour_to_int(self.__choosecolour))
            self.status_buffer.send()

    def __stop_choosing(self):
        self.keyfunctor.clear_gfunctor()

    def __unchoose(self,subject):
        self.__stop_choosing()
        self.__update_lights()

    def __add_choices(self):
        if self.__choosemusical:
            new = []
            newpos = []
            for choice in self.__choices:
                pos = (int(choice[1][0]),int(choice[1][1]))
                newpos.append(pos)
                new.append([pos,self.__choosecolour])
            existing = [x for x in logic.parse_clause(self.get_musical()) if x[0] not in newpos]
            existing.extend(new)
            self.set_musical_map(logic.render_term(existing))
        else:
            new = []
            newpos = []
            for choice in self.__choices:
                pos = (int(choice[0][0]),int(choice[0][1]))
                newpos.append(pos)
                new.append([pos,self.__choosecolour])
            existing = [x for x in logic.parse_clause(self.get_physical()) if x[0] not in newpos]
            existing.extend(new)
            self.set_physical_map(logic.render_term(existing))


agent.main(Agent)

