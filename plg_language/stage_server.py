
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

#---------------------------------------------------------------------------
#
# XMLRPC server for Stage-EigenD backend
#
#---------------------------------------------------------------------------

from SimpleXMLRPCServer import SimpleXMLRPCServer
from SimpleXMLRPCServer import SimpleXMLRPCRequestHandler
import sys
import xml.dom.pulldom
import xml.dom.minidom
import threading
import piw
from pi import agent,atom,domain,errors,action,bundles,async,utils,resource,logic,node,upgrade,const,paths
import traceback
import widget
import language_native
import socket

def log_error(func):
    def __f(*args,**kwds):
        try:
            return func(*args,**kwds)
        except:
            traceback.print_exc(limit=None)
            raise
        return __f



#---------------------------------------------------------------------------
# XMLRPC server functions
#---------------------------------------------------------------------------

class stageXMLRPCFuncs:

    def __init__(self, languageAgent):
        # the tab atoms stored in the interpreter
        self.__languageAgent = languageAgent
        self.__tabs = languageAgent.tabs
        self.__database = languageAgent.database
        self.__widgetManager = languageAgent.widgets
        #self.loadAgents('/Users/arran/git/stage/xmlrpc/testXMLRPC/agents.xml')
        #self.loadTabs('/Users/arran/git/stage/xmlrpc/testXMLRPC/tabs.xml')

        self.setupName = 'setup'
        # dict for agent xml
        self.agents = {}
        # dict for atoms enabled for use in Stage
        self.enabledAtoms = {}
        
        # session ID is the current timestamp
        self.__sessionID = str(piw.tsd_time())
        
        # last time the agents where updated
        self.__last_agent_update_time = 0
        
        self.__last_agentIDs = frozenset()

    #---------------------------------------------------------------------------
    # aux functions
    #---------------------------------------------------------------------------

    def __getNameOrdinalPair(self, words):
        
        wordsList = words.split()
        # name can be anything, but check for an integer in last element and handle it like an ordinal
        if wordsList==[]:
            return('', None)
        last = wordsList[len(wordsList)-1]
        if last.isdigit():
            # strip off ordinal
            name = ' '.join(wordsList[:len(wordsList)-1])
            ordinal = int(last)
            return (name, ordinal)
        else:
            return (words, None)

    def __getStringFromNameOrdinalPair(self, pair):
        # add the ordinal to the name string
        (name, ordinal) = pair
        if ordinal!=None:
            return name+' '+str(ordinal)
        else:
            return name

    def __atomHelpXml(self,atomID):
        atomHelp,localHelp = self.__languageAgent.find_help(atomID)

        if localHelp:
            tip_txt = [localHelp]
            hlp_txt = [localHelp]
        else:
            tip_txt = []
            hlp_txt = []

        if atomHelp:
            generic_hlp = atomHelp.get_helptext()
            generic_tip = atomHelp.get_tooltip()
            if generic_hlp: hlp_txt = [generic_hlp]+hlp_txt
            if generic_tip: tip_txt = [generic_tip]+tip_txt

        xml = ''

        if tip_txt:
            xml += '<tip>%s</tip>\n' % ' '.join(tip_txt)

        if hlp_txt:
            xml += '<help>%s</help>\n' % ' '.join(hlp_txt)

        return xml
                    

    def __buildAtomsXml(self, atomIDs, oscPath):
        # make xml for a tree of atoms
        
        xml = ''

        # list of (agent ID, (name, ordinal))
        atoms = [(atomID, self.__getNameOrdinalPair(self.__database.find_desc(atomID))) for atomID in atomIDs]

        # sort by name-ordinal pair to make sure numbers sorted in right order
        atoms.sort(key=lambda x:x[1])

        # walk through atoms
        for atom in atoms:
            atomName = self.__getStringFromNameOrdinalPair(atom[1])

            # recurse into child atoms
            childAtomIDs = self.__database.find_children(atom[0])

            # format xml according to whether this is a leaf atom
            if len(childAtomIDs)>0:
                #print 'atom props:',self.__database.get_propcache('props').get_valueset(atom[0])
                
                # ignore child nodes that are agents, e.g. in rigs
                if 'agent' not in self.__database.get_propcache('props').get_valueset(atom[0]):
                    if atomName != None and atomName != '':
                        xml += ('<atom name="%s">\n'%atomName + 
                                self.__buildAtomsXml(childAtomIDs, (oscPath+'/'+atomName.replace(' ','_'))) + 
                                self.__atomHelpXml(atom[0]) +
                                '</atom>\n')
                    else:
                        # collapse unnamed node
                        xml += self.__buildAtomsXml(childAtomIDs, oscPath)
            else:
                # child node
                # ignore atoms with no name
                if atomName != None and atomName != '':

                    # ignore child nodes that are agents, e.g. in rigs
                    if 'agent' not in self.__database.get_propcache('props').get_valueset(atom[0]):

                        # extract domain
                        atomDomain = self.__database.find_item(atom[0]).domain()
                        atomDomainCanonical = atomDomain.canonical()
                        atomDomainType = atomDomainCanonical[0:atomDomainCanonical.find('(')]

                        cache = self.__database.get_propcache('protocol')
                        noStage = 'nostage' in cache.get_valueset(atom[0])


                        isNumericType = atomDomainType=='bint' or \
                                           atomDomainType=='bfloat' or \
                                           atomDomainType=='bintn' or \
                                           atomDomainType=='bfloatn'
                        isSupportedType = (isNumericType or atomDomainType=='bool' or atomDomainType=='trigger') and not noStage

                        #print oscPath,atomName,atomDomainCanonical,noStage,isSupportedType

                        atomPath = (oscPath+'/'+atomName.replace(' ','_'))

                        xml += '<atom name="%s" '%atomName
                        xml += 'path="%s" '%atomPath
                        xml += 'address="%s" '%atom[0]
                        
                        # indicate if Stage can use this atom value, show in tree as 'enabled'
                        if(isSupportedType):
                            xml += 'enabled="true" '
                        else:
                            xml += 'enabled="false" '
                        
                        xml += '>\n'

                        xml += '<domain type="%s" '%atomDomainType

                        # numeric domain range
                        if isNumericType:
                            min = atomDomain.min
                            max = atomDomain.max
                            xml += 'min="%f" '%min+'max="%f" '%max

                        xml += '/>\n'

                        xml += self.__atomHelpXml(atom[0])

                        xml += '</atom>'

                        # store enabled atom addresses for use by the widget manager
                        if(isSupportedType):
                            self.enabledAtoms[atomPath] = atom[0]
                    
        
        return xml

    def __buildAgentsXml(self, agentIDs):
        #print 'build agents xml'
        # make xml for a set of agents in the database 

        # TODO: this does not need to iterate over a list, could just do one agent
        
        try:
        
            #agentIDs = self.__database.get_propcache('props').get_idset('agent')

            xml = ''

            # list of (agent ID, (name, ordinal))
            agents = [(agentID, self.__getNameOrdinalPair(self.__database.find_desc(agentID))) for agentID in agentIDs]
            # sort by name-ordinal pair to make sure numbers sorted in right order
            agents.sort(key=lambda x:x[1])
            
            for agent in agents:
                agentName = self.__getStringFromNameOrdinalPair(agent[1])

                # if agent has a name it exists
                if agentName != '':
                    # child atoms, union of child atoms and subsystems
                    childAtomIDs = list(self.__database.find_children(agent[0]).union(self.__database.find_joined_slaves(agent[0])))

                    # store the agent subtree xml strings
                    self.agents[agent[0]] = ('<agent name="%s" '%agentName + 'address="%s">\n'%agent[0] + 
                                             self.__buildAtomsXml(childAtomIDs, ('/'+agentName.replace(' ','_'))) + 
                                             self.__atomHelpXml(agent[0]) +
                                             '</agent>\n')

            return True

        except:
            traceback.print_exc(limit=None)
            return False

    def __buildAgentXml(self, agentID):
        #print 'build agent xml'
        # make xml for an agent in the database 

        try:
        
            xml = ''

            agentName = self.__database.find_desc(agentID)

            # if agent has a name it exists
            if agentName != '':
                # child atoms, union of child atoms and subsystems
                childAtomIDs = list(self.__database.find_children(agentID).union(self.__database.find_joined_slaves(agentID)))

                # store the agent subtree xml strings
                self.agents[agentID] = ('<agent name="%s" '%agentName + 'address="%s">\n'%agentID + 
                                         self.__buildAtomsXml(childAtomIDs, ('/'+agentName.replace(' ','_'))) + 
                                         self.__atomHelpXml(agentID) +
                                         '</agent>\n')

            return True

        except:
            traceback.print_exc(limit=None)
            return False

    def execBelcanto(self, argument):
        piw.tsd_lock()

        r = self.__languageAgent.rpc_script(argument)

        if r.status() is not None:
            piw.tsd_unlock()
            return r.status()

        args = [None,None,None]
        e = threading.Event()
        e.clear()

        def finished(returnstatus,*returnargs,**returnkwds):
            args[0] = returnstatus
            args[1] = returnargs
            args[2] = returnkwds
            e.set()

        r.setCallback(finished,True).setErrback(finished,False)
        piw.tsd_unlock()
        e.wait()
        return args[0]


    #---------------------------------------------------------------------------
    # global rpcs
    #
    # used by stage to get information about eigenD
    #---------------------------------------------------------------------------

    def ping(self):
        # a client can test if the server is connected by calling this
        print 'ping...'
        return True

    def getSessionID(self):
        #print 'get session id = ',self.__sessionID
        return self.__sessionID

    def getSetupName(self):
        return self.setupName

    def getOSCPort(self):
        print 'get OSC port'
        try:
            return self.__languageAgent.widgets.get_server_port()
        except:
            traceback.print_exc(limit=None)
            return ""
            

    #---------------------------------------------------------------------------
    # agent rpcs
    # 
    # used by Stage to get information about the setups agents and atoms
    #---------------------------------------------------------------------------
        
    def getAgentChangeSet(self, clientTimeStr):
        #print 'get agent change set'
        # get the set of agent IDs for the agents that have been updated
        # since the last time the client requested changes
        try:
            piw.tsd_lock()

            #if fromStartup:
            #    time = 0
            #else:
            #    time = self.__last_agent_update_time

            clientTime = long(clientTimeStr)
            #print 'client time str = ',clientTimeStr

            changed_agent_ids = self.__database.changed_agents(clientTime)

            if len(changed_agent_ids)>0:
                agentIDs = self.__database.get_propcache('props').get_idset('agent')

                # only interested in changed agents (roots)
                changed_agent_ids = agentIDs.intersection(frozenset(changed_agent_ids))

                # remove any agents that have been removed from the database
                removed_agentIDs = self.__last_agentIDs - agentIDs
                for id in removed_agentIDs:
                    del self.agents[id]

                # add removed agents to the change list so Stage will attempt to
                # get deleted agents, and can determine that they have been
                # deleted when nothing is returned
                changed_agent_ids = changed_agent_ids.union(removed_agentIDs)

                self.__last_agentIDs = agentIDs

                piw.tsd_unlock()

                # build xml list of changed agent list
                xml = '<agentChangeSet time="%s">\n' % self.__database.get_timestamp()

                for id in changed_agent_ids:
                    xml += '<agent address="%s"/>\n' % id

                xml += '</agentChangeSet>'

                #if len(changed_agent_ids)>0:
                #    print 'number changes=',len(changed_agent_ids),'time =',clientTime
                #    print 'changed set',changed_agent_ids
                #    print 'removed set',removed_agentIDs
                #    print xml

            else:
            
                piw.tsd_unlock()
            
                xml = '<agentChangeSet time="%s"></agentChangeSet>' % self.__database.get_timestamp()


            #self.__last_agent_update_time = self.__database.get_timestamp()

            return xml


        except:
            piw.tsd_unlock()
            traceback.print_exc(limit=None)

            return ''

    def getAgent(self, agentID):
        #print 'get agent ',agentID
        # return xml for an agent with a given ID
        self.__buildAgentXml(agentID)
        if agentID in self.agents:        
            return self.agents[agentID]
        else:
            # if the agent does not exist, then return empty string to tell
            # Stage to forget about it
            return ''


        
    #---------------------------------------------------------------------------
    # TODO: remove test functions

    def getNumAgents(self):
        print 'get num agents =',len(self.agents)
        return len(self.agents)

    def getAgentID(self, name):
        print 'get agent ID',name
        ID = self.__database.find_by_name(name)
        print ID
        return ID

    def setAgent(self, index):
        return True

    def loadAgents(self, filename):
        print 'load agents'
        # load agents xml to make a dictionary of agents
        agentsFile = open(filename, 'r')
        
        doc = xml.dom.pulldom.parse(agentsFile)

        self.agents = {}
        index = 0
        for event, node in doc:
            if event=='START_ELEMENT' and node.nodeName=='agent':
                doc.expandNode(node)
                # store the agent subtree xml strings
                self.agents[index] = node.toxml()
                index+=1
            elif event=='START_ELEMENT' and node.nodeName=='setup':
                self.setupName = node.getAttribute('name')
                print "     setup name =",self.setupName
           

        agentsFile.close()

        print "     loaded %s agents"%len(self.agents)
        
        return True

    def saveAgents(self, filename):
        try:
            print 'save agents'
            doc = '<?xml version="1.0" ?>\n'
            doc += '<agents>\n'

            piw.tsd_lock()
            self.getAgentChangeSet(True)
            agents = [(agentID, self.__getNameOrdinalPair(self.__database.find_desc(agentID))) for agentID in self.__last_agentIDs]
            # sort by name-ordinal pair to make sure numbers sorted in right order
            agents.sort(key=lambda x:x[1])
            piw.tsd_unlock()
            
            for (id, (n,o)) in agents:
                doc += self.getAgent(id)

            doc += '</agents>\n'

            print doc

            tabsFile = open(filename, 'w')
            tabsFile.write(doc)
            tabsFile.close()

            print "     saved %s agents"%self.getNumAgents()
        except:
            traceback.print_exc(limit=None)
            return False

        return True



    #---------------------------------------------------------------------------
    # tab rpcs
    #
    # used by Stage to store and retrieve the tabs to and from the setup
    #---------------------------------------------------------------------------

    def getNumTabs(self):
        try:
            piw.tsd_lock()
            numTabs = len(self.__tabs)
            piw.tsd_unlock()
            #print 'get num tabs =',numTabs
        except:
            traceback.print_exc(limit=None)
            return 0
        return numTabs
        
    def addTab(self, tabXML):
        try:
            print 'add tab'
            # receive new tab xml from stage for an empty tab - no widgets to add to manager
            piw.tsd_lock()
            # tab added with index numTabs
            self.__tabs.new_tab(tabXML)
            piw.tsd_unlock()
        except:
            traceback.print_exc(limit=None)
            return False
        return True

    def setTab(self, index, tabXML):
        try:
            print 'set tab',index
            piw.tsd_lock()
            self.__tabs.get_tab(index).setXML(tabXML)
            piw.tsd_unlock()
        except:
            traceback.print_exc(limit=None)
            return False
        return True

    def getTab(self, index):
        # return tab xml to stage
        try:
            print 'get tab',index
            piw.tsd_lock()
            tabXML = self.__tabs.get_tab(index).getXML()
            piw.tsd_unlock()
        except:
            traceback.print_exc(limit=None)
            return ''
        # return xml string
        return tabXML

    def moveTab(self, currentTabIndex, newTabIndex):
        # moves a tab (swaps two tabs)
        try:
            print 'move tab', currentTabIndex, 'to', newTabIndex
            piw.tsd_lock()
            # current and new must be valid tab indices
            self.__tabs.move_tab(currentTabIndex, newTabIndex);
            piw.tsd_unlock()
        except:
            traceback.print_exc(limit=None)
            return False
        return True

    def removeTab(self, tabIndex):
        # removes a tab
        try:
            print 'remove tab', tabIndex

            # remove all the widgets from this tab
            # ensures that they are removed from the widget manager also
            numWidgets = self.getNumWidgets(tabIndex)
            for widgetIndex in range(0,numWidgets):
                # always remove widget 0 as the list gets shorter
                self.removeWidget(tabIndex,0)
            
            piw.tsd_lock()
            self.__tabs.remove_tab(tabIndex)
            piw.tsd_unlock()

        except:
            traceback.print_exc(limit=None)
            return False
        return True

    def getTabSessionChanges(self, index):
        # return the number of tab session changes
        # each time a tab is changed, the session change count is incremented
        try:
            piw.tsd_lock()
            tab = self.__tabs.get_tab(index)
            if tab is not None:
                changes = tab.get_session_changes()
            else:
                changes = -1
            piw.tsd_unlock()
            #print 'get tab session changes index =',index,'changes =',changes
            return changes
        except:
            traceback.print_exc(limit=None)
            return -1
       


    #---------------------------------------------------------------------------
    # widget rpcs
    #
    # used by stage to store and retrieve the widgets to and from the setup
    #---------------------------------------------------------------------------
    
    def getNumWidgets(self, tabIndex):
        # get number of widgets in a tab
        try:
            piw.tsd_lock()
            numWidgets = len(self.__tabs.get_tab(tabIndex)[1])
            piw.tsd_unlock()
            print 'get num widgets =',numWidgets
        except:
            traceback.print_exc(limit=None)
            return 0
        return numWidgets
    
    def addWidget(self, tabIndex, widgetXML):
        print 'add widget to tab',tabIndex
        # add widget to tab
        try:
            piw.tsd_lock()
            widgets = self.__tabs.get_tab(tabIndex)[1]

            widgets.new_widget(widgetXML)
            piw.tsd_unlock()

            # parse widget xml to get node
            widgetDoc = xml.dom.minidom.parseString(widgetXML)
            widgetNode = widgetDoc.documentElement

            # create widget in widget manager
            name = widgetNode.getAttribute('name')
            OSCPath = widgetNode.getAttribute('path')
            if OSCPath!='':
                self.__widgetManager.create_widget(OSCPath)
                print "created widget",name,OSCPath
            else:
                return False
        except:
            traceback.print_exc(limit=None)
            return False
        return True

    def setWidget(self, tabIndex, widgetIndex, widgetXML):
        print 'set widget',widgetIndex,'from tab',tabIndex
        # set widget xml
        try:
            piw.tsd_lock()
            widgets = self.__tabs.get_tab(tabIndex)[1]
            widgets.get_widget(widgetIndex).setXML(widgetXML)
            piw.tsd_unlock()
        except:
            traceback.print_exc(limit=None)
            return ''
        return widgetXML
                
                                
    def getWidget(self, tabIndex, widgetIndex):
        print 'get widget',widgetIndex,'from tab',tabIndex
        # get widget xml
        try:
            piw.tsd_lock()
            widgets = self.__tabs.get_tab(tabIndex)[1]
            widgetXML = widgets.get_widget(widgetIndex).getXML()
            piw.tsd_unlock()
        except:
            traceback.print_exc(limit=None)
            return ''
        return widgetXML
        
    def removeWidget(self, tabIndex, widgetIndex):
        # remove widget with index
        print 'remove widget',widgetIndex,'from tab',tabIndex
        try:
            piw.tsd_lock()
            widgetXML = self.__tabs.get_tab(tabIndex)[1].get_widget(widgetIndex).getXML()
            widgetDoc = xml.dom.minidom.parseString(widgetXML)
            widgetNode = widgetDoc.documentElement
            OSCPath = widgetNode.getAttribute('path')
            self.__tabs.get_tab(tabIndex)[1].remove_widget(widgetIndex)
            piw.tsd_unlock()

            # remove widget from widget manager, reference count deletion in destroy_widget
            if OSCPath!='':
                self.__widgetManager.destroy_widget(OSCPath)
            else:
                return False
        except:
            traceback.print_exc(limit=None)
            return False
        return True

    def reconnectWidgets(self):
        # TODO: remove this if not needed
        # reconnect widgets by their OSC path name
        # used after importing widgets from another setup
        print 'reconnect widgets'
        try:
            self.__languageAgent.widgets.reconnect_widgets()
        except:
            traceback.print_exc(limit=None)
            return False
        return True


    def updateAllWidgetPaths(self, changedNodes):
        # update all widgets with a set of addresses mapping to new OSC paths
        # called by the widget manager when agents and atoms are renamed in eigenD
        #print 'updateWidgetPaths'
        try:
            numTabs = len(self.__tabs)
            # search all widgets in all tabs for widgets matching address
            for tabIndex in range(0,numTabs):
                widgets = self.__tabs.get_tab(tabIndex)[1]
                numWidgets = len(widgets)
                for widgetIndex in range(0,numWidgets):
                    widgetXML = widgets.get_widget(widgetIndex).getXML()
                    widgetDoc = xml.dom.minidom.parseString(widgetXML)
                    widgetNode = widgetDoc.documentElement
                    widgetAddress = widgetNode.getAttribute('address')
                    if widgetAddress in changedNodes:
                        # update the widget xml with the new osc path
                        newOSCPath = changedNodes[widgetAddress]
                        #print '    updated widget path',newOSCPath
                        widgetNode.setAttribute('path', newOSCPath)
                        widgetDoc.documentElement = widgetNode
                        widgetXML = widgetDoc.documentElement.toxml()
                        widgets.get_widget(widgetIndex).setXML(widgetXML)
        except:
            traceback.print_exc(limit=None)
            return False
        #print 'updateWidgetPaths done'
        return True



    #---------------------------------------------------------------------------
    # TODO: remove test functions

    def removeAllTabs(self):
        print 'remove all tabs'
        self.__tabs.clear()
        return True

    def removeAllWidgets(self):
        print 'remove all widgets'
        try:
            # remove all current widget controllers
            self.__widgetManager.clear()
        except:
            traceback.print_exc(limit=None)
            return False
        return True
    
    def dumpTabs(self):
        print 'dump tabs'
        print self.__tabs
        return True

    def loadTabs(self, filename):
        print 'load tabs'
        # load tabs to make a list of tabs
        tabsFile = open(filename, 'r')
        
        doc = xml.dom.pulldom.parse(tabsFile)

        self.tabs = {}
        index = 0
        for event, node in doc:
           if event=='START_ELEMENT' and node.nodeName=='tab':
               doc.expandNode(node)
               # store the tab subtree xml strings
               self.tabs[index] = node.toxml()
               index+=1

        tabsFile.close()

        print "     loaded %s tabs"%len(self.tabs)
        
        return True

    def saveTabs(self, filename):
        try:
            print 'save tabs'
            doc = '<?xml version="1.0" ?>\n'
            doc += '<tabs>\n'

            for i in range(0,self.getNumTabs()):
                tab = (self.getTab(i))
                doc += tab[0:len(tab)-3]+'>\n'
                for j in range(0, self.getNumWidgets(i)):
                    doc += self.getWidget(i,j)

                doc += '</tab>\n'

            doc += '</tabs>\n'

            print doc

            tabsFile = open(filename, 'w')
            tabsFile.write(doc)
            tabsFile.close()

            print "     saved %s tabs"%self.getNumTabs()
        except:
            traceback.print_exc(limit=None)
            return False

        return True

       
#-------------------------------------------------------------------------------        
# Stage XMLRPC server thread
#-------------------------------------------------------------------------------        

# TODO: set IP address and socket

class RequestHandler(SimpleXMLRPCRequestHandler):
    rpc_paths = ('/RPC2',)

class stageXMLRPCServer(threading.Thread):
    def __init__(self, languageAgent, snapshot, xmlrpc_server_port):
        print "stage __init__"
        self.__languageAgent = languageAgent
        self.snapshot = snapshot
        self.xmlrpc_server_port = xmlrpc_server_port

        threading.Thread.__init__(self)
        self.timeToQuit = threading.Event()
        self.timeToQuit.clear()      

    def stop(self):    
        print "Shutting down stage xmlrpc server"
        self.server.server_close()
        self.timeToQuit.set()

    def run(self):
        print "Running stage xmlrpc server"

        self.snapshot.install()

        self.server = SimpleXMLRPCServer(("0.0.0.0", self.xmlrpc_server_port), requestHandler=RequestHandler, logRequests=False)

        self.server.register_introspection_functions()

        # advertise service through Bonjour
        self.__bonjour = language_native.bonjour(self.xmlrpc_server_port)
        self.__bonjour.service_register()
        
        # Register an instance; all the methods of the instance are
        # published as XML-RPC methods
        self.server_instance = stageXMLRPCFuncs(self.__languageAgent)
        self.server.register_instance(self.server_instance)
        self.__languageAgent.widgets.set_stage_server(self.server_instance)

        # run loop
        while not self.timeToQuit.isSet():
            try:
                self.server.handle_request()
            except socket.error, e:
                print 'Stage server error: xmlrpc server raised socket error ',e
            except:
                print 'Stage server error: unexpected error'



#-------------------------------------------------------------------------------        
# Widget atom for storing widget xml
#-------------------------------------------------------------------------------        

class Widget(atom.Atom):
    def __init__(self, tab):
        atom.Atom.__init__(self,domain=domain.String(),policy=atom.default_policy(self.setXML),protocols='nostage')
        self.tab = tab
   
    def setXML(self,xml):
        self.set_value(xml)
        self.tab.inc_session_changes()
        return False

    def getXML(self):
        xml = self.get_value()
        return xml



#-------------------------------------------------------------------------------        
# Widget list atom for storing widget atoms in the setup
#
# stored within the tab atoms
#-------------------------------------------------------------------------------        

class WidgetList(atom.Atom):

    def __init__(self, tab):
        atom.Atom.__init__(self,creator=self.__create_widget,wrecker=self.__wreck_widget,protocols='nostage')
        self.tab = tab

    def __create_widget(self, index):
        #print 'WidgetList __create widget',index
        newWidget = Widget(self.tab)
        self.tab.inc_session_changes()
        return newWidget
        
    def __wreck_widget(self, index, node):
        #print 'WidgetList __wreck_widget',index
        del self[index]
        self.tab.inc_session_changes()

    def new_widget(self, xml):
        newWidgetIndex = len(self)
        atomIndex = self.find_hole()
        newWidget = Widget(self.tab)
        newWidget.setXML(xml)
        newWidget.set_property_long('widgetIndex',newWidgetIndex)
        self[atomIndex] = newWidget

    def get_widget(self, widgetIndex):
        for (atomIndex,atom) in self.iteritems():
            if atom.get_property_long('widgetIndex')==widgetIndex:
                return atom
                break
        return None
        
    def remove_widget(self,widgetIndex):
        for (atomIndex,atom) in self.iteritems():
            index = atom.get_property_long('widgetIndex')
            if index==widgetIndex:
                delAtomIndex = atomIndex
            else:
                if index>widgetIndex:
                    newWidgetIndex = atom.get_property_long('widgetIndex')-1
                    atom.set_property_long('widgetIndex', newWidgetIndex)
        del self[delAtomIndex]
        self.tab.inc_session_changes()



#-------------------------------------------------------------------------------        
# Tab atom for storing tab xml
#-------------------------------------------------------------------------------        

class Tab(atom.Atom):
    def __init__(self,agent,sessionChanges):
        self.__agent = agent
        self.__sessionChanges = sessionChanges
        
        atom.Atom.__init__(self,domain=domain.String(),policy=atom.default_policy(self.setXML),protocols='nostage')
        self.set_property_long('tabIndex',-1)

        self[1] = WidgetList(self)

    def setXML(self,xml):
        self.set_value(xml)
        self.inc_session_changes()
        return False

    def getXML(self):
        xml = self.get_value()
        return xml
    
    def inc_session_changes(self):
        index = self.get_property_long('tabIndex')
        if index != -1:
            if index in self.__sessionChanges:
                self.__sessionChanges[index] = self.__sessionChanges[index]+1
            else:
                self.__sessionChanges[index] = 0
        else:
            print 'TabError: inc_session_changes has no index'
            print self
        
    def get_session_changes(self):
        index = self.get_property_long('tabIndex')
        if index != -1:
            #print 'sessionChange[',index,']=',self.__sessionChanges[index]
            return self.__sessionChanges[index]
        else:
            return 0



#-------------------------------------------------------------------------------        
# Tab list atom for storing tab atoms in the setup
#
# stored in a langauge agent atom
#-------------------------------------------------------------------------------        

class TabList(atom.Atom):
    def __init__(self,agent):
        self.__agent = agent
        self.__sessionChanges = {}
        atom.Atom.__init__(self,creator=self.__create_tab,wrecker=self.__wreck_tab,protocols='nostage')
        
    def __create_tab(self, index):
        #print 'TabList __create_tab',index
        newTab = Tab(self.__agent,self.__sessionChanges)
        return newTab
    
    def __wreck_tab(self, index, node):
        #print 'TabList __wreck_tab',index
        
        del self[index]

    def new_tab(self, xml):
        newTabIndex = len(self)
        atomIndex = self.find_hole()
        newTab = Tab(self.__agent,self.__sessionChanges)
        newTab.set_property_long('tabIndex',newTabIndex)
        self.__sessionChanges[newTabIndex] = 0
        newTab.setXML(xml)
        self[atomIndex] = newTab

    def get_tab(self, tabIndex):
        for (atomIndex,atom) in self.iteritems():
            if atom.get_property_long('tabIndex')==tabIndex:
                return atom
        return None
        
    def remove_tab(self,tabIndex):
        for (atomIndex,atom) in self.iteritems():
            index = atom.get_property_long('tabIndex')
            if index==tabIndex:
                delAtomIndex = atomIndex
            else:
                if index>tabIndex:
                    newTabIndex = atom.get_property_long('tabIndex')-1
                    atom.set_property_long('tabIndex', newTabIndex)
                    self.__sessionChanges[newTabIndex] = self.__sessionChanges[index]
                    
        del self[delAtomIndex]

    def move_tab(self,currentTabIndex,newTabIndex):
        currentAtom = None
        newAtom = None
        for (atomIndex,atom) in self.iteritems():
            index = atom.get_property_long('tabIndex')
            if index==currentTabIndex:
                currentAtom = atom
            if index==newTabIndex:
                newAtom = atom
        if currentAtom is not None and newAtom is not None:
            currentAtom.set_property_long('tabIndex',newTabIndex)
            newAtom.set_property_long('tabIndex',currentTabIndex)
            newSessionChanges = max(self.__sessionChanges[currentTabIndex], self.__sessionChanges[newTabIndex])+1
            self.__sessionChanges[currentTabIndex] = newSessionChanges
            self.__sessionChanges[newTabIndex] = newSessionChanges
        else:
            print 'TabList error: could not move tabs'


























