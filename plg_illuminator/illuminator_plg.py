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

from pi import agent,atom,domain,policy,bundles,logic,action,utils
from . import illuminator_version as version

import piw
import threading,sys,socket,fileinput,re
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer

MATCH_PHYSICAL = re.compile('/row/(\d+)/column/(\d+)',re.IGNORECASE)
MATCH_MUSICAL = re.compile('/course/(\d+)/key/(\d+)',re.IGNORECASE)
MATCH_MAP = re.compile('\[(?:\[\[\d+,\d+\],\w+\](?:,\[\[\d+,\d+\],\w+\])*)?\]',re.IGNORECASE)

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
    def output_get(self, content):
        self.send_response(200)
        self.send_header("Content-Type", "text/plain")                     
        self.end_headers()
        self.wfile.write(content)

    def do_GET(self):
        if self.path == '/physical':
            self.output_get(self.server.agent.get_physical())
            return

        if self.path == '/musical':
            self.output_get(self.server.agent.get_musical())
            return

        self.send_response(404)
        self.end_headers()

    def do_PUT(self):
        if self.path == '/physical':
            content = self.rfile.read(int(self.headers['Content-Length']))
            if MATCH_MAP.match(content):
                self.send_response(200)
                self.end_headers()
                self.server.agent.set_physical_map(content)
            else:
                self.send_response(400)
                self.end_headers()
            return

        match = MATCH_PHYSICAL.match(self.path)
        if match:
            self.send_response(200)
            self.end_headers()
            row = int(match.group(1))
            column = int(match.group(2))
            colour = self.rfile.read(int(self.headers['Content-Length']))
            self.server.agent.set_physical(row,column,colour)
            return

        match = MATCH_MUSICAL.match(self.path)
        if match:
            self.send_response(200)
            self.end_headers()
            course = int(match.group(1))
            key = int(match.group(2))
            colour = self.rfile.read(int(self.headers['Content-Length']))
            self.server.agent.set_musical(course,key,colour)
            return

        self.send_response(404)
        self.end_headers()

    def do_DELETE(self):
        if self.path == '/':
            self.send_response(200)
            self.end_headers()
            self.server.agent.clear()
            return

        if self.path == '/physical':
            self.send_response(200)
            self.end_headers()
            self.server.agent.clear_physical()
            return

        if self.path == '/musical':
            self.send_response(200)
            self.end_headers()
            self.server.agent.clear_musical()
            return

        match = MATCH_PHYSICAL.match(self.path)
        if match:
            self.send_response(200)
            self.end_headers()
            row = int(match.group(1))
            column = int(match.group(2))
            self.server.agent.unset_physical(row,column)
            return

        match = MATCH_MUSICAL.match(self.path)
        if match:
            self.send_response(200)
            self.end_headers()
            course = int(match.group(1))
            key = int(match.group(2))
            self.server.agent.unset_musical(course,key)
            return

        self.send_response(404)
        self.end_headers()

class Agent(agent.Agent):
    def __init__(self, address, ordinal):
        agent.Agent.__init__(self, signature=version, names='illuminator', ordinal=ordinal)

        self.domain = piw.clockdomain_ctl()

        self[1] = bundles.Output(1, False, names='light output',protocols='revconnect')
        self.output = bundles.Splitter(self.domain, self[1])

        self.status_buffer = piw.statusbuffer(self.output.cookie())
        self.status_buffer.autosend(False)
 
        self[2] = atom.Atom(domain=domain.String(), init='[]', names='physical light map', policy=atom.default_policy(self.__physical_light_map))
        self[3] = atom.Atom(domain=domain.String(), init='[]', names='musical light map', policy=atom.default_policy(self.__musical_light_map))
        self[5] = atom.Atom(domain=domain.BoundedIntOrNull(0,65535,0), names='server port', policy=atom.default_policy(self.__server_port))

        self.keyfunctor = piw.functor_backend(1, False)
        self.keyinput = bundles.VectorInput(self.keyfunctor.cookie(), self.domain, signals=(1,))
        self[4] = atom.Atom(domain=domain.Aniso(), policy=self.keyinput.vector_policy(1,False), names='key input')
        self.choicefunctor = utils.make_change_nb(piw.slowchange(utils.changify(self.__choice)))

        self.add_verb2(1,'clear([],None)', callback=self.__clear)
        self.add_verb2(2,'clear([],None,role(None,[matches([physical])]))', callback=self.__clear_physical)
        self.add_verb2(3,'clear([],None,role(None,[matches([musical])]))', callback=self.__clear_musical)
        self.add_verb2(4,'set([],None,role(None,[coord(physical,[row],[column])]),role(to,[abstract,matches([red])]))', callback=self.__set_physical)
        self.add_verb2(5,'set([],None,role(None,[coord(physical,[row],[column])]),role(to,[abstract,matches([green])]))', callback=self.__set_physical)
        self.add_verb2(6,'set([],None,role(None,[coord(physical,[row],[column])]),role(to,[abstract,matches([orange])]))', callback=self.__set_physical)
        self.add_verb2(7,'set([],None,role(None,[coord(musical,[course],[key])]),role(to,[abstract,matches([red])]))', callback=self.__set_musical)
        self.add_verb2(8,'set([],None,role(None,[coord(musical,[course],[key])]),role(to,[abstract,matches([green])]))', callback=self.__set_musical)
        self.add_verb2(9,'set([],None,role(None,[coord(musical,[course],[key])]),role(to,[abstract,matches([orange])]))', callback=self.__set_musical)
        self.add_verb2(10,'set([un],None,role(None,[coord(physical,[row],[column])]))', callback=self.__unset_physical)
        self.add_verb2(11,'set([un],None,role(None,[coord(musical,[course],[key])]))', callback=self.__unset_musical)
        self.add_verb2(12,'choose([],None,role(None,[matches([physical])]),role(as,[abstract,matches([red])]))',self.__choose_physical)
        self.add_verb2(13,'choose([],None,role(None,[matches([physical])]),role(as,[abstract,matches([green])]))',self.__choose_physical)
        self.add_verb2(14,'choose([],None,role(None,[matches([physical])]),role(as,[abstract,matches([orange])]))',self.__choose_physical)
        self.add_verb2(15,'choose([],None,role(None,[matches([musical])]),role(as,[abstract,matches([red])]))',self.__choose_musical)
        self.add_verb2(16,'choose([],None,role(None,[matches([musical])]),role(as,[abstract,matches([green])]))',self.__choose_musical)
        self.add_verb2(17,'choose([],None,role(None,[matches([musical])]),role(as,[abstract,matches([orange])]))',self.__choose_musical)
        self.add_verb2(18,'choose([un],None)',self.__unchoose)

        self.httpServer = None

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

    def close_server(self):
        agent.Agent.close_server(self)
        self.stop_http()
 
    def __physical_light_map(self,v):
        self.set_physical_map(v)

    def __musical_light_map(self,v):
        self.set_musical_map(v)

    def __server_port(self,v):
        self[5].set_value(v)

        if v:
            self.start_http()
        else:
            self.stop_http()

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
        row,column = action.coord_value(key)
        colour = action.abstract_string(colour)
        self.set_physical(row,column,colour)

    def set_physical(self,row,column,colour):
        phys = [x for x in logic.parse_clause(self.get_physical()) if x[0][0] != row or x[0][1] != column]
        phys.append([[row,column],colour])
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
        row,column = action.coord_value(key)
        self.unset_physical(row,column)

    def unset_physical(self,row,column):
        phys = [x for x in logic.parse_clause(self.get_physical()) if x[0][0] != row or x[0][1] != column]
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
                self.status_buffer.set_status(musical,int(m[0][0]),int(m[0][1]),self.__colour_to_int(m[1]))

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
        if not choice[4]: return
        # remove the hardness so that it's not part of the identity of the choice
        del choice[4]

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
                key = choice[3]
            else:
                key = choice[1]
            self.status_buffer.set_status(self.__choosemusical,key[0],key[1],self.__colour_to_int(self.__choosecolour))
            self.status_buffer.send()

    def __stop_choosing(self):
        self.keyfunctor.clear_gfunctor()

    def __unchoose(self,subject):
        self.__stop_choosing()
        self.__update_lights()

    def __add_choices(self):
        if self.__choosemusical:
            existing = list(logic.parse_clause(self.get_musical()))
            for choice in self.__choices:
                existing.append([[int(choice[3][0]),int(choice[3][1])],self.__choosecolour])
            self.set_musical_map(logic.render_term(existing))
        else:
            existing = list(logic.parse_clause(self.get_physical()))
            for choice in self.__choices:
                existing.append([[int(choice[1][0]),int(choice[1][1])],self.__choosecolour])
            self.set_physical_map(logic.render_term(existing))


agent.main(Agent)

