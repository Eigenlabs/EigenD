
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

from pi import atom,policy,utils,proxy,const,domain,logic,rpc
import language_native
import piw
import xmlrpclib

server_port = "55551"

monitor_debug = False
widget_debug = False
widget_manager_debug = False

class Monitor(proxy.AtomProxy):
    def __init__(self,recv_channel,osc,widget):
        proxy.AtomProxy.__init__(self)
        self.__anchor = piw.canchor()
        self.__anchor.set_client(self)
        self.__recv = recv_channel
        self.__fast = None
        self.__osc = osc
        self.__widget = widget
        self.target_id = None

    def set_target(self,target_id):
        if monitor_debug:
            print '-   Monitor: set_target',target_id,' - unplumbing first...'
        self.unplumb()
        self.__anchor.set_address_str(target_id or '')
        self.target_id = target_id

    def node_ready(self):
        if monitor_debug:
            print '-   Monitor: node_ready'
        self.plumb()

    def node_removed(self):
        if monitor_debug:
            print '-   Monitor: node_removed'
        self.unplumb()

    def plumb(self):
        if monitor_debug:
            print '-       Monitor: plumb - unplumbing first...'
        self.unplumb()
        if monitor_debug:
            print '-       Monitor: plumbing...'

        if self.is_fast():
            if monitor_debug:
                print '-       Monitor: plumbed fast'
            self.set_sink(self.__recv)
            self.__fast = True
        else:
            if monitor_debug:
                print '-       Monitor: not plumbed fast'
            self.__aux = piw.fastdata(const.fastdata_sender)
            piw.tsd_fastdata(self.__aux)
            self.__recv.set_upstream(self.__aux)
            self.__auxq = piw.tsd_dataqueue(16)
            self.__aux.send_slow(piw.pathnull(piw.tsd_time()),self.__auxq)
            self.set_change_handler(self.__auxq.sender_slow())
            data = self.get_data()
            if not data.is_null():
                if monitor_debug:
                    print 'target=',self.target_id,'data="',self.get_data(),'" ready=',self.is_ready(),'type=',type(data)
                self.__auxq.sender_slow()(data)
            self.__fast = False

        # called when a connection can successfully be made
        if self.__widget.send_channel is not None:
            if monitor_debug:
                print '-       Monitor: set_connected true',self.__widget.send_channel
            self.__osc.set_connected(self.__widget.send_channel, True)
        else:
            if monitor_debug:
                print '-       Monitor: **** not set_connected ****'
        # try sending change in status
        osc_path = self.__widget.get_property_string('target-name')
        if osc_path!='':
            if monitor_debug:
                print '-       Monitor: sent osc connected 1'
            self.__osc.send(osc_path+'/connected',1)

    def unplumb(self):
        if monitor_debug:
            print '-       Monitor: unplumb',self.__anchor.get_address()
                
        if self.__fast is True:
            self.clear_sink()
            self.__fast = None

        if self.__fast is False:
            self.clear_change_handler()
            self.__recv.clear_upstream()
            self.__aux.close_fastdata()
            self.__aux = None
            self.__auxq = None
            self.__fast = None

        # called when the port being controlled goes away
        if self.__widget.send_channel is not None:
            if monitor_debug:
                print '-       Monitor: set_connected false',self.__widget.send_channel
            self.__osc.set_connected(self.__widget.send_channel, False)
        else:
            if monitor_debug:
                print '-       Monitor: **** not set_connected ****'
        # try sending change in status
        osc_path = self.__widget.get_property_string('target-name')
        if osc_path!='':
            if monitor_debug:
                print '-       Monitor: send osc connected 0'
            self.__osc.send(osc_path+'/connected',0)
        

class Widget(atom.Atom):
    def __init__(self,osc):
        atom.Atom.__init__(self,policy=policy.FastReadOnlyPolicy(),domain=domain.Aniso(),protocols='nostage')
        self.__send_queue = piw.fastdata(0)
        self.__recv_queue = piw.fastdata(0)
        piw.tsd_fastdata(self.__send_queue)
        piw.tsd_fastdata(self.__recv_queue)
        self.send_channel = None
        self.__monitor = Monitor(self.__recv_queue,osc,self)
        self.get_policy().set_source(self.__send_queue)
        self.__osc = osc
        self.__name = ''
        self.have_target = False
        self.have_widget = False

    def property_change(self,key,value):
        if widget_debug:
            print 'Widget: property_change',key,value,'is open=',self.open()
        if self.open():
            if key == 'target-id':
                # need to make sure we have a widget first so wait until target-name is set
                if self.have_widget:
                    # set target which will plumb if possible
                    self.__monitor.set_target(value.as_string())
                    self.have_target = True

            if key == 'target-name':
                self.have_widget = True
                name = value.as_string()
                if self.__name != name:
                    if self.send_channel is not None:
                        if widget_debug:
                            print '-   Widget: delete OSC widget, name=',self.__name,'send_channel=',self.send_channel
                        self.__osc.del_widget(self.send_channel)
                    #self.__osc.del_widget_by_name(value.as_string())
                    self.send_channel = self.__osc.add_widget(value.as_string(),self.__recv_queue,self.__send_queue)
                    self.__name = name
                    if widget_debug:
                        print '-   Widget: create OSC widget, name=',self.__name,'send_channel=',self.send_channel
                    
                # target was waiting for widget so create one now
                if not self.have_target:
                    self.__monitor.set_target(self.get_property_string('target-id'))

    def server_opened(self):
        atom.Atom.server_opened(self)
        name = self.get_property_string('target-name')
        id = self.get_property_string('target-id')
        if widget_debug:
            print 'Widget: open widget server',name,id
        if name and id:
            if self.__name != name:
               if self.send_channel is not None:
                    if widget_debug:
                        print '-   Widget: delete OSC widget, name=',self.__name,'send_channel=',self.send_channel
                    self.__osc.del_widget(self.send_channel)
               self.send_channel = self.__osc.add_widget(name,self.__recv_queue,self.__send_queue)
               self.__name = name
               if widget_debug:
                   print '-   Widget: create OSC widget, name=',self.__name,'send_channel=',self.send_channel
            self.have_widget = True

            # target set which will plumb if possible
            self.__monitor.set_target(id)
            self.have_target = True

    def close_server(self):
        if widget_debug:
            print 'Widget: close widget server',self.get_property_string('target-name'),self.get_property_string('target-id')
        atom.Atom.close_server(self)
        self.__monitor.set_target('')
        if self.send_channel is not None:
            self.__osc.del_widget(self.send_channel)
            self.send_channel = None
            self.__name = ''
        self.have_target = False
        self.have_widget = False
        if widget_debug:
            print 'Widget: close widget server count=',self.get_property_long('ref-count')
            
    def destroy(self):
        # remove this widget from the targets controllers
        cs = logic.render_term(logic.make_term('ctl',self.id(),None))
        address = self.get_property_string('target-id')
        if address!='':
            rpc.invoke_rpc(address,'uncontrol',cs)
    
    def setup(self,address,name):
        self.set_property_long('ref-count',1)
        self.set_property_string('target-name',name)
        self.set_property_string('target-id',address)
        cs = logic.render_term(logic.make_term('ctl',self.id(),None))
        if address!='':
            return rpc.invoke_rpc(address,'control',cs)
        else:
            return False

class WidgetManager(atom.Atom):
    def __init__(self, language_agent, xmlrpc_server_port):
        atom.Atom.__init__(self,creator=self.__create,wrecker=self.__wreck,bignode=True,protocols='nostage')
        self.__osc = language_native.oscserver(server_port, str(xmlrpc_server_port))
        self.__server_port = server_port
        self.__stage_server_instance = None
        self.__database = language_agent.database
    
    def set_stage_server(self, stage_server_instance):
        self.__stage_server_instance = stage_server_instance

    def server_opened(self):
        atom.Atom.server_opened(self)
        self.__osc.startup()

    def close_server(self):
        atom.Atom.close_server(self)
        self.__osc.shutdown()

    def __create(self,index):
        if widget_manager_debug:
            print 'WidgetManager: __create',index
        return Widget(self.__osc)

    def __wreck(self,index,node):
        if widget_manager_debug:
            print 'WidgetManager: __wreck',index
        pass

    def create_widget(self,name = None):
        if self.__stage_server_instance is not None:
            if name in self.__stage_server_instance.enabledAtoms:
                address = self.__stage_server_instance.enabledAtoms[name]
            else:
                # no address so this atom does not exist in this setup (e.g. when importing tabs)
                address = ''
            if widget_manager_debug:
                print 'WidgetManager: create_widget name =',name,' address=',address
            # reference count number of widgets with a particular OSC path (target-name)
            # create by the current osc path, associate with the current target address
            for (k,v) in self.iteritems():
                if v.get_property_string('target-name') == name:
                    count = v.get_property_long('ref-count')
                    current_address = v.get_property_string('target-id')
                    # does current address exist anymore?
                    if address!=current_address:
                        if widget_manager_debug:
                            print 'WidgetManager: changing address',current_address,'->',address
                        # change address, remove current controller
                        v.destroy()
                        # set new address and new controller
                        v.setup(address,name)
                    v.set_property_long('ref-count', count+1)
                    return True
            
            i = self.find_hole()
            self[i] = Widget(self.__osc)
            return self[i].setup(address,name or '/widget%d' % i)
        else:
            if widget_manager_debug:
                print 'WidgetManager: could not create widget name =',name
            return False


    def destroy_widget(self,name):
        if widget_manager_debug:
            print 'WidgetManager: destroy_widget'
        for (k,v) in self.iteritems():
            # destroy by it's OSC path (target-name)
            if v.get_property_string('target-name') == name:
                count = v.get_property_long('ref-count')
                if(count-1==0):
                    self[k].destroy()
                    del self[k]
                else:
                    v.set_property_long('ref-count',count-1)
                return True
        return False


    def get_server_port(self):
        return self.__server_port


    def check_widget_name_updates(self, changed_nodes):
        if widget_manager_debug:
            print 'WidgetManager: check_widget_name_updates'
        #print 'changed_nodes:',changed_nodes
        if self.__stage_server_instance is not None:
            new_names = {}
            # update names in widgets
            for (k,v) in self.iteritems():
                address = v.get_property_string('target-id')
                if address in changed_nodes:
                        new_name = self.__stage_server_instance.build_osc_name(address)
                        if widget_manager_debug:
                            print 'WidgetManager: changing widget name', address, new_name
                        # set name in widget
                        v.set_property_string('target-name',new_name)
                        # set the new osc widget as connected since plumbing not changed
                        self.__osc.set_connected(v.send_channel, True)
                        new_names[address] = new_name

            # update new names in tabs
            if len(new_names)>0:
                self.__stage_server_instance.updateAllWidgetPaths(new_names)

        else:
            print 'WidgetManager: warning: when checking widget name updates, stage server not defined'

        if widget_manager_debug:
            print 'WidgetManager: check_widget_name_updates done'
