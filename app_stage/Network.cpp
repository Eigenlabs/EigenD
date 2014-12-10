/*
 Copyright 2010-2014 Eigenlabs Ltd.  http://www.eigenlabs.com

 This file is part of EigenD.

 EigenD is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 EigenD is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Network.h"
#include <signal.h>
#include <time.h>
#include "WidgetComponent.h"

#if STAGE_BUILD==IOS
#include "iOSReachability.h"
#endif // STAGE_BUILD

#define OSC_DEBUG 0
#define OSC_RENDEZVOUS_PORT "55554"

#define STATUS_DISCONNECT 0
#define STATUS_CONNECTING 1
#define STATUS_CONNECTED 2

//------------------------------------------------------------------------------
//
// Connection Manager
//
// separate thread to manage the connection to the eigenD XMLRPC and OSC servers
// supports 2 discovery protocols:
// 1. Bonjour
// 2. OSC rendezvous
//------------------------------------------------------------------------------

ConnectionManager::ConnectionManager(XMLRPCManager* manager): Thread("eigenD connection manager"), xmlrpcManager_(manager), rendezvousServer_(0), listener_(0)
{
    initialize();
}

ConnectionManager::~ConnectionManager()
{
    shutdown();
}

void ConnectionManager::initialize()
{
    // OSC rendezvous
    rendezvousServer_ = lo_server_new("55550", staticErrorHandler);
    
    if(rendezvousServer_)
    {
        lo_server_add_method(rendezvousServer_, "/eigend", "si", staticRendezvousHandler, (void*)this);
        serverFileDescriptor_ = lo_server_get_socket_fd(rendezvousServer_);
#if STAGE_BUILD==IOS
        int on = 1;
        setsockopt(serverFileDescriptor_, SOL_SOCKET, SO_NOSIGPIPE, (void *)&on, sizeof(on));
#endif // STAGE_BUILD
        startThread();
    }
    else
    {
        std::cout << "error: failed to create OSC rendezvous server\n";
    }
}

void ConnectionManager::shutdown()
{
    stopThread(10000);
    
    if(rendezvousServer_)
    {
        lo_server_free(rendezvousServer_);
        rendezvousServer_ = 0;
    }
}

void ConnectionManager::run()
{
    // separate thread that will reconnect Stage to the xmlrpc server, osc server
    // by discovering eigenD servers by OSC rendezvous
    // also performs Bonjour discovery processing when data appears on the mDNSResponder sockets

    while (!threadShouldExit()) 
    {
        // broadcast OSC rendezvous to discover available eigenD XMLRPC servers
        if(rendezvousServer_)
        {
            discover();

            // receive OSC rendezvous callbacks from eigenD
            for(unsigned count=0;count<20;count++)
            {
                lo_server_recv_noblock(rendezvousServer_, 10);
            }
        }
        
        detectRemovedEigenDHosts();
        wait(1000);
    }
    
}

//------------------------------------------------------------------------------
// OSC Rendezvous functions
//------------------------------------------------------------------------------

void ConnectionManager::discover()
{
    // broadcast message to find eigend servers

    // clear existing servers
    
    //std::cout << "sending to eigend service discovery multicast server\n";
    
    lo_message message = lo_message_new();
    
    lo_address broadcast_addr = lo_address_new("255.255.255.255", OSC_RENDEZVOUS_PORT);

    int error = lo_send_message_from_broadcast(broadcast_addr, rendezvousServer_, "/eigend", message);
    if (error==-1)
    {
        std::cout << "lo discovery broadcast send error " << error << "\n";
    }
    
    lo_address local_addr = lo_address_new("127.0.0.1", OSC_RENDEZVOUS_PORT);
    
    error = lo_send_message_from(local_addr, rendezvousServer_, "/eigend", message);
    if (error==-1)
    {
        std::cout << "lo discovery local send error " << error << "\n";
    }
}

int ConnectionManager::rendezvousHandler(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg)
{
    // handler responds to /eigend messages in response to broadcast
    
    //printf("path: <%s>=%s\n", path, &argv[0]->s);
    
    String strPath = String(path);
    
    // reply from OSC based eigend xmlrpc server service discovery
    lo_address srcAddr = lo_message_get_source(msg);
    const char *srcAddrChar = lo_address_get_hostname(srcAddr);
    
    String url(&argv[0]->s);
    
    int colon = url.indexOfChar(':');
    String addrStr = url.substring(0,colon);
    String portStr = url.substring(colon+1);
    
    //std::cout << "found eigend through OSC addr=" << srcAddrChar << ":" << portStr << " name=" << addrStr << " loopback=" << argv[1]->i << "\n";
    
    addEigenDHost(addrStr);
    HostTarget target(addrStr);
    target.address = String(srcAddrChar);
    target.port = (unsigned)portStr.getIntValue();
    target.loopback = argv[1]->i;
    setEigenDHostTarget(addrStr, target);

    return 1;
    
}

int ConnectionManager::staticRendezvousHandler(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data)
{
    return ((ConnectionManager*)user_data)->rendezvousHandler(path, types, argv, argc, msg);
}

void ConnectionManager::staticErrorHandler(int num, const char *msg, const char *path)
{
    // error handler
    printf("Connection manager OSC server error %d in path %s: %s\n", num, path, msg);
}

void ConnectionManager::hostListChanged()
{
    ScopedLock l(hostListCriticalSection_);
    
    if(listener_)
    {
        //std::cout << "posting update message\n";
        listener_->postMessage(new Message());
    }
}

void ConnectionManager::setMessageListener(MessageListener *m)
{
    ScopedLock l(hostListCriticalSection_);
    listener_=m;
}

void ConnectionManager::clearMessageListener(MessageListener *m)
{
    ScopedLock l(hostListCriticalSection_);
    
    if(listener_==m)
    {
        listener_ = 0;
    }
}

void ConnectionManager::addEigenDHost(String& serviceName)
{
    ScopedLock l(hostListCriticalSection_);
    
    // initialize host in map
    if (eigenDHosts_.count(serviceName)==0)
        eigenDHosts_[serviceName] = HostTarget(serviceName);

}

void ConnectionManager::setEigenDHostTarget(String& serviceName, HostTarget& target)
{
    // only set the port if the host had been added
    // so that the port information of a removed host does not recreate it

    hostListCriticalSection_.enter();
    
    if(eigenDHosts_.count(serviceName)>0)
    {
        if(eigenDHosts_[serviceName].address == "")
        {
            // if new host then add the target
            //std::cout << "add new host "<<serviceName<<"\n";
            eigenDHosts_[serviceName] = target;
            hostListChanged();
        }
        else
        {
            if(target.loopback==1)
            {
                if(eigenDHosts_[serviceName].loopback == 0)
                //std::cout << "localhost found "<<serviceName<<"\n";
                // this target was found through loopback so it's the localhost
                // so overwrite this name with the localhost address
                eigenDHosts_[serviceName] = target;
                hostListChanged();
            }

        }

        eigenDHosts_[serviceName].foundCount = 3;
        
    }

    hostListCriticalSection_.exit();
    
    // update the list box

}

void ConnectionManager::removeEigenDHost(const String& serviceName)
{
    {
        ScopedLock l(hostListCriticalSection_);
        //std::cout << "remove host " << serviceName << "\n";
        eigenDHosts_.erase(serviceName);
    }

    hostListChanged();

}

void ConnectionManager::detectRemovedEigenDHosts()
{
    ScopedLock l(hostListCriticalSection_);

    std::map<String, HostTarget>::iterator i, j;
    i=eigenDHosts_.begin();

    while (i!=eigenDHosts_.end()) 
    {
        j=i;
        i++;
        if((*j).second.foundCount-- < 0)
            removeEigenDHost((*j).first);
    }
    
}


//------------------------------------------------------------------------------
// ListBoxModel functions
//------------------------------------------------------------------------------

unsigned ConnectionManager::getNumHosts()
{
    ScopedLock l(hostListCriticalSection_);
    return eigenDHosts_.size();
}

HostTarget ConnectionManager::getHost(unsigned hostNumber)
{
    ScopedLock l(hostListCriticalSection_);
    std::map<String, HostTarget>::iterator i;
    unsigned count = 0;

    for (i=eigenDHosts_.begin(); i!=eigenDHosts_.end(); i++) 
    {
        if (hostNumber==count)
        {
            return i->second;
        }
            
        count++;
    }

    return HostTarget("");
}

//------------------------------------------------------------------------------
//
// Bonjour callback functions
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// XMLRPC Manager
//
//------------------------------------------------------------------------------

XMLRPCManager::XMLRPCManager(MainComponent* mainComponent): mainComponent_(mainComponent), client_(0), connection_(0),
    status_(STATUS_DISCONNECT), browser_disabled_(true), agentViewUpdate_(false), statusDialog_(0)
{
#ifndef WIN32
    // prevent xmlrpc's throwing SIGPIPE signals
    //signal(SIGPIPE, SIG_IGN);
	struct sigaction mySigAction;
    mySigAction.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &mySigAction, NULL);
    sigemptyset(&mySigAction.sa_mask);
#endif
    
}

XMLRPCManager::~XMLRPCManager()
{
    shutdown();
    
    if(client_)
        deleteAndZero(client_);
    
}

void XMLRPCManager::shutdown()
{
    if(statusDialog_)
    {
        statusDialog_->exitModalState(-1);
        statusDialog_ = 0;
    }
    
    stopTimer();
    shutdownThreads();
    status_ = STATUS_DISCONNECT;
}

void XMLRPCManager::disableBrowser(bool b)
{
    browser_disabled_ = b;

    if(!b)
    {
        startTimer(0);
    }
}

void XMLRPCManager::shutdownThreads()
{
    if(connection_)
    {
        if(connection_->isThreadRunning())
            connection_->stopThread(5000);
        deleteAndZero(connection_);
    }
}

void XMLRPCManager::initialize()
{
    // start up connection manager thread
    connection_ = new ConnectionManager(this);
    stopTimer();
}

void XMLRPCManager::suspend()
{
    shutdown();
    if(mainComponent_ && mainComponent_->getOSCManager())
    {
        mainComponent_->getOSCManager()->shutdown();
    }
}

void XMLRPCManager::resume()
{
    if(status_ != STATUS_DISCONNECT)
    {
        return;
    }
    
    initialize();
    if(mainComponent_ && mainComponent_->getOSCManager())
    {
        mainComponent_->getOSCManager()->initialize();
        mainComponent_->getOSCManager()->requestAllWidgetValues();

    }
    
    if(!browser_disabled_ || agentViewUpdate_)
    {
        startTimer(0);
    }
}

bool XMLRPCManager::execute(const char *method, XmlRpcValue const &params, XmlRpcValue &result, bool retry)
{
    // handle execute here so connection failures can be dealt with
    bool hasResponded = false;
    bool isFault = true;

restart:
    
    if (status_ == STATUS_CONNECTING)
    {
        throw XMLRPCAbortException();
    }

    if (status_ == STATUS_CONNECTED)
    {
        try
        {
            //std::cout << "xmlrpc execute " << method << "(" << params.toXml() << ")\n";
            // have 500ms time out for ipod so doesn't stall if network connection lost
            //hasResponded = client_->execute(method, params, result, 0.5);
            // make it behave like desktop until connections preference button is put in
            hasResponded = client_->execute(method, params, result, 4.0);
            isFault = client_->isFault();
        }
        catch(XmlRpcException e)
        {
            std::cout << "xmlrpc execute exception " << e.getMessage() << "\n";
        }
        catch (...) 
        {
            // something went wrong
            std::cout << "exception during xmlrpc execute\n";
        }
        
        // connected, but a fault occurred
        if (hasResponded && !isFault)
        {
            return true;
        }
            
        // eigenD connection lost, try and re-establish
        if(!hasResponded)
            std::cout << "xmlrpc server did not respond\n";
        if(isFault)
            std::cout << "xmlrpc server fault\n";

        // start reconnection process as XMLRPC server lost
        status_ = STATUS_DISCONNECT;
    }

    if(browser_disabled_)
    {
        throw XMLRPCAbortException();
    }
    
    if(statusDialog_)
    {
        statusDialog_->exitModalState(-1);
    }
        
    StatusDialogComponent* statusDialog = new StatusDialogComponent();
    statusDialog_ = statusDialog;
    
    status_ = STATUS_CONNECTING;

    statusDialog->addToDesktop(0);

    if(desired_host_.isEmpty())
    {
        statusDialog->setTitle("Connect to eigenD");
#if STAGE_BUILD==IOS
        if(iOSReachability::isReachableForLocalWifi())
        {
            statusDialog->setStatusMessage("Searching the network for eigenD, make sure you're connected to the same network ...");
        }
        else
        {
            statusDialog->setStatusMessage("Please enable WIFI on your device, this is required to be able to detect eigenD on your network ...");
        }
#else
        statusDialog->setStatusMessage("Searching the local machine and network for eigenD servers...");
#endif // STAGE_BUILD
    }
    else
    {
        statusDialog->setTitle("Reconnecting or select other EigenD");
#if STAGE_BUILD==IOS
        if(iOSReachability::isReachableForLocalWifi())
        {
            statusDialog->setStatusMessage("Reconnecting to eigenD, make sure you're connected to the same network ...");
        }
        else
        {
            statusDialog->setStatusMessage("Please enable WIFI on your device, this is required to be able to reconnect to eigenD ...");
        }
#else
        statusDialog->setStatusMessage("Reconnecting to eigenD...");
#endif // STAGE_BUILD
    }

#if STAGE_BUILD==DESKTOP
    statusDialog->showQuitButton(true);
#else
    statusDialog->showQuitButton(false);
#endif // STAGE_BUILD==DESKTOP

    statusDialog->setUpPosition(mainComponent_,320, 224);
    statusDialog->setDesiredService(desired_host_);
    statusDialog->showConnectButton(true);
    statusDialog->getConnectButton()->setEnabled(false);
    statusDialog->getAutoToggleButton()->setToggleState(mainComponent_->preferences_->getBoolValue("connectSingleHost", false), dontSendNotification);
    statusDialog->showProgressBar(false);
    statusDialog->showStatusMessage(true);
    statusDialog->showHostList(false);
    statusDialog->showAutoToggleButton(true);
    statusDialog->setConnectionManager(connection_);

    std::cout << "entering modal " << method << "\n";
    int dialog_result = statusDialog->runModalLoop();
    std::cout << "exiting modal " << method << ':' << dialog_result << "\n";
    statusDialog->removeFromDesktop();

    mainComponent_->preferences_->setValue("connectSingleHost", statusDialog->getAutoToggleButton()->getToggleState());
    mainComponent_->preferences_->saveIfNeeded();
    
    delete statusDialog;
    if(statusDialog_ == statusDialog)
    {
        statusDialog_ = 0;
    }

    if(dialog_result<0)
    {
#if STAGE_BUILD==DESKTOP
        // quit button pressed in modal network dialog, post message to main window to shutdown
        mainComponent_->getMainWindow()->postMessage(new Message());
#endif // STAGE_BUILD==DESKTOP
        throw XMLRPCAbortException();
    }

    // connect to selected host
    HostTarget t = connection_->getHost(dialog_result);

    if(!t.name.isEmpty() && t.port && !t.address.isEmpty())
    {
        if(connect(t.address,t.port))
        {
            desired_host_ = t.name;
        }
    }

    if(retry)
        goto restart;

    return false;
}

bool XMLRPCManager::connect(String &hostName, unsigned port)
{
    // connect to host port
    
    // message thread xmlrpc client
    if(client_)
        deleteAndZero(client_);
    
    std::cout << "connecting to " << hostName << " port " << port << std::endl;

    client_ = new XmlRpcClient(hostName.toUTF8(), (int)port, "/RPC2");

    // determine the OSC port in the eigenD server and connect the OSC server

    XmlRpcValue params, result;
    
    bool hasResponded = false;
    bool isFault = false;
    
    params.clear();
    
    try 
    {
		hasResponded = client_->execute("getOSCPort", params, result);
        isFault = client_->isFault();
    }
    catch(XmlRpcException e)
    {
        std::cout << "xmlrpc server getOSCPort execute exception " << e.getMessage() << std::endl;
    }
    catch (...) 
    {
        // something went wrong
        std::cout << "exception during xmlrpc server getOSCPort execute" << std::endl;
    }
    
    bool isOK = (hasResponded && !isFault);
    
    if(isOK)
    {
        std::string osc_port = std::string(result);
        isOK = mainComponent_->getOSCManager()->connect(hostName,osc_port.c_str());
        status_ = STATUS_CONNECTED;
        return true;
    }
    else
    {
        status_ = STATUS_DISCONNECT;
        return false;
    }

}

void XMLRPCManager::browseForEigenD()
{
    desired_host_ = "";
    status_ = STATUS_DISCONNECT;
    stopTimer();
    startTimer(0);
}

void XMLRPCManager::timerCallback()
{
    stopTimer();
    postMessage(new Message());
}

void XMLRPCManager::handleMessage(const Message &message)
{
#if STAGE_BUILD==DESKTOP
    if(agentViewUpdate_)
        mainComponent_->getAgentViewComponent()->receiveAgentChanges();
#endif // STAGE_BUILD==DESKTOP
    
    cout << "EigenD Ping\n";
    mainComponent_->getTabbedComponent()->receiveTabChanges();
    startTimer(2000);
}

void XMLRPCManager::setAgentViewUpdate(bool agentViewUpdate)
{
    agentViewUpdate_ = agentViewUpdate;
    
    if(agentViewUpdate_)
    {
        timerCallback();
    }
}


//------------------------------------------------------------------------------
// String conversions
//------------------------------------------------------------------------------
String XMLRPCManager::stdToJuceString(const std::string& stdString)
{
    return String::fromUTF8(stdString.c_str());
}

std::string XMLRPCManager::juceToStdString(const String& juceString)
{
    return std::string(juceString.toUTF8());
}



//------------------------------------------------------------------------------
//
// OSC Manager
//
//------------------------------------------------------------------------------

OSCManager::OSCManager(XMLRPCManager* xmlrpcManager) : Thread("OSCManager"), xmlrpcManager_(xmlrpcManager), eigenDAddr_(0), server_(0)
{
}

OSCManager::~OSCManager()
{
    shutdown();
}

void OSCManager::timerCallback() 
{
    cout << "ping EigenD\n";
    if (eigenDAddr_)
    {
        lo_send_from(eigenDAddr_, server_, LO_TT_IMMEDIATE, "/ping", "");
    }

    startTimer(40000);
}

bool OSCManager::initialize()
{
    server_ = 0;
    
    // server here refers to Stage being the OSC 'server'
    // all OSC applications are servers, both Stage and EigenD
    server_ = lo_server_new("55552", staticErrorHandler);
    
    std::cout << "creating osc server url: " << lo_server_get_url(server_) << "\n";
    
    if(server_)
    {
        lo_server_add_method(server_, NULL, NULL, staticGenericHandler, (void*)this);
        serverFileDescriptor_ = lo_server_get_socket_fd(server_);
#if STAGE_BUILD==IOS
        int on = 1;
        setsockopt(serverFileDescriptor_, SOL_SOCKET, SO_NOSIGPIPE, (void *)&on, sizeof(on));
#endif // STAGE_BUILD==IOS
        startTimer(40000);
        startThread();
        
        return true;
    }
    else
    {
        std::cout << "failed to create osc server\n";
        
        return false;
    }
}

String OSCManager::getHostName()
{
    // determine the network name of the host from the OSC server
    String hostname;

#if STAGE_BUILD==DESKTOP
    // use hostname on desktop machines
    if (server_)
        hostname = XMLRPCManager::stdToJuceString(std::string(lo_url_get_hostname(lo_server_get_url(server_))));
#else
    // use mac address on iOS machines
    // NOTE: can use hostname on iOS, don't need to use the MAC address
    //       - it is the machine name e.g. Eigenlabs-iPod
    Array<MACAddress> macAddresses;
    MACAddress::findAllAddresses(macAddresses);
    // should be only one mac address...
    if (macAddresses.size()>0)
    {
        hostname = macAddresses[0].toString();
    }
#endif // STAGE_BUILD==DESKTOP
    return hostname;
}

void OSCManager::shutdown()
{
    stopTimer();
    stopThread(10000);
    
    if(eigenDAddr_)
    {
        lo_address_free(eigenDAddr_);
        eigenDAddr_ = 0;
    }
    
    if(server_)
    {
        lo_server_free(server_);
        server_ = 0;
    }
    
}

bool OSCManager::connect(const String &host,const char* port)
{
    stopTimer();
    stopThread(10000);

    // connect to OSC host address
    if(eigenDAddr_)
    {
        lo_address_free(eigenDAddr_);
        eigenDAddr_ = 0;
    }

    if(!port || !port[0])
    {
        std::cout << "no osc destination port available, failed to connect to osc address\n";
        return false;
    }
    else
    {
        std::cout << "connecting to OSC address name=" << host << " port=" << port << "\n";
        
        eigenDAddr_ = lo_address_new(host.toUTF8(), port);
        
        //return true;
    }

    startThread();
    startTimer(0);
    return true;
}
    


void OSCManager::sendString(String path, const String &value) const
{
#if OSC_DEBUG==1
    std::cout << "send path: <" << path << ">\narg 0 's' " << value << "\n\n";
#endif // OSC_DEBUG==1
    
    if(eigenDAddr_)
    {
        const char *s = value.toUTF8();
        lo_send(eigenDAddr_, path.toUTF8(), "s", s);
    }


}


void OSCManager::sendBlob(String path, const void *value, unsigned size) const
{
#if OSC_DEBUG==1
    std::cout << "send path: <" << path << ">\narg 0 'b' " << size << "\n\n";
#endif // OSC_DEBUG==1
    
    if(eigenDAddr_)
    {
        lo_blob blob = lo_blob_new(size,value);
        lo_send(eigenDAddr_, path.toUTF8(), "b", blob);
    }


}

void OSCManager::sendFloat(String path, float value) const
{
#if OSC_DEBUG==1
    std::cout << "send path: <" << path << ">\narg 0 'f' " << value << "\n\n";
#endif // OSC_DEBUG==1
    
    if(eigenDAddr_)
        lo_send(eigenDAddr_, path.toUTF8(), "f", value);


}

void OSCManager::sendValue(String path, const WidgetData &value) const
{
    if(value.isFloat())
    {
        sendFloat(path,value.float_);
        return;
    }

    if(value.isString())
    {
        sendString(path,value.string_);
        return;
    }

    if(value.isBlob())
    {
        sendBlob(path,value.blob_.getData(),value.blob_.getSize());
        return;
    }
}


void OSCManager::run()
{
    // run server thread
    while (!threadShouldExit()) 
    {
        lo_server_recv_noblock(server_, 10);
    }
}





int OSCManager::genericHandler(const char *path, const char *types, lo_arg **argv,
                                     int argc, lo_message msg)
{
    // generic (catch all) OSC message handler

#if OSC_DEBUG==1
    int i;

    printf("    recv path: <%s>\n", path);
    for (i=0; i<argc; i++) 
    {
        printf("    arg %d '%c' ", i, types[i]);
        lo_arg_pp((lo_type)types[i], argv[i]);
        printf("\n");
    }
    printf("\n");
    fflush(stdout);
#endif // OSC_DEBUG==1
    
    // if path ends in echo, then strip echo
    String strPath = String(path);

    bool isConnectedMessage = strPath.endsWith("/connected");
    if (isConnectedMessage)
    {
        strPath = strPath.dropLastCharacters(10);
    }
    
    {
        ScopedLock l(widgetMapCriticalSection_);
    
        // lookup widgets from path
        if(widgetMap_.count(strPath)!=0)
        {
            // send message to all widgets on this path
            set<WidgetComponent*>::iterator iter;
            set<WidgetComponent*>* widgets = widgetMap_[strPath];

            for (iter=widgets->begin(); iter!=widgets->end(); iter++)
            {
                if(isConnectedMessage)
                {
                    (*iter)->postMessage(new WidgetConnectedMessage(argv[0]->f==1.0));
                }
                else
                {
                    if(types[0]=='f')
                    {
                        (*iter)->postMessage(new WidgetValueMessage(argv[0]->f));
                    }
                    else if(types[0]=='b')
                    {
                        (*iter)->postMessage(new WidgetValueMessage(lo_blob_dataptr(argv[0]),lo_blob_datasize(argv[0])));
                    }
                    else if(types[0]=='s')
                    {
                        (*iter)->postMessage(new WidgetValueMessage(&(argv[0]->s)));
                    }
                }
            }
        }
    }

    return 1;

    
}

int OSCManager::staticGenericHandler(const char *path, const char *types, lo_arg **argv,
                                    int argc, lo_message msg, void *user_data)
{
    return ((OSCManager*)user_data)->genericHandler(path, types, argv, argc, msg);
}

void OSCManager::staticErrorHandler(int num, const char *msg, const char *path)
{
    // error handler
    printf("OSC server error %d in path %s: %s\n", num, path, msg);
}


//------------------------------------------------------------------------------
// widget management functions
//------------------------------------------------------------------------------

// TODO: - remove empty path?
//       - delete sets on map destruction

void OSCManager::addWidgetToPath(const String& path, WidgetComponent* widget)
{
    widgetMapCriticalSection_.enter();
    
    //const char* str = path.toUTF8();
    
    if(widgetMap_.count(path)!=0)
    {
        // widget controlling an existing path
        widgetMap_[path]->insert(widget);
    }
    else 
    {
        // widget controlling a new path
        set<WidgetComponent*>* widgetSet = new set<WidgetComponent*>();
        widgetSet->insert(widget);
        
        widgetMap_.insert(pair<const String, set<WidgetComponent*>* >(path, widgetSet));
    }

    widgetMapCriticalSection_.exit();
    
}

void OSCManager::removeWidgetFromPath(const String& path, WidgetComponent* widget)
{
    widgetMapCriticalSection_.enter();
    
    if(widgetMap_.count(path)!=0)
    {
        widgetMap_[path]->erase(widget);
    }    

    widgetMapCriticalSection_.exit();
    
}

void OSCManager::requestAllWidgetValues()
{
    // request the values of the widget atoms
    // by sending a null data item to all paths

    widgetMapCriticalSection_.enter();
    
    if(eigenDAddr_)
    {
        map< const String, set<WidgetComponent*>* >::iterator iter;
        
        for(iter=widgetMap_.begin(); iter!=widgetMap_.end(); iter++)
        {
            
            const char* path = (*iter).first.toUTF8();
#if OSC_DEBUG==1
            cout << "requesting (all values)" << path << "\n";
#endif // OSC_DEBUG==1            
            lo_send_from(eigenDAddr_, server_, LO_TT_IMMEDIATE, path, "");
        }
        
    }

    widgetMapCriticalSection_.exit();
}


void OSCManager::requestWidgetValue(const String& path)
{
    // request the values of the widget atoms
    // by sending a null data item to a path

    if (eigenDAddr_)
    {    
        const char* pathChar = path.toUTF8();
#if OSC_DEBUG==1
        cout << "requesting " << pathChar << "\n";
#endif // OSC_DEBUG==1        
        lo_send_from(eigenDAddr_, server_, LO_TT_IMMEDIATE, pathChar, "");
        
    }
    
}
