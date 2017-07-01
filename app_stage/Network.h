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

#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "lib_lo/lo/lo.h"
#include "lib_xmlrpc++/XmlRpc.h"
using namespace XmlRpc;

#include "juce.h"


#include <map>
#include <set>
#include <list>
using namespace std;



class MainComponent;
class WidgetComponent;
class XMLRPCManager;
class WidgetData;
class StatusDialogComponent;


//------------------------------------------------------------------------------
//
// HostTarget
//
// IP address as a DNS name and port number of a host target
//------------------------------------------------------------------------------

struct HostTarget 
{
    HostTarget(): port(0) {}
    HostTarget(const String &n): name(n), port(0) {}
    HostTarget(const HostTarget &h): name(h.name), address(h.address), port(h.port), loopback(h.loopback), foundCount(h.foundCount) {}
    HostTarget &operator=(const HostTarget &h) { name=h.name; address=h.address; port=h.port; loopback=h.loopback; foundCount=h.foundCount; return *this; }

    String name;
    String address;
    unsigned port;
    bool loopback;
    int foundCount;
};

//------------------------------------------------------------------------------
//
// Connection Manager
//
// separate thread to manage the connection to the eigenD XMLRPC and OSC servers
//------------------------------------------------------------------------------

class ConnectionManager : public Thread
{
public:
    ConnectionManager(XMLRPCManager* manager);
    ~ConnectionManager();

    void run();
    void shutdown();
    void initialize();

    //--------------------------------------------------------------------------
    // OSC rendezvous

    void discover();
    
    int rendezvousHandler(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg);
    static int staticRendezvousHandler(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data);
    static void staticErrorHandler(int num, const char *msg, const char *path);

    //--------------------------------------------------------------------------
    // eigenD host management

    void addEigenDHost(String& serviceName);
    void setEigenDHostTarget(String& serviceName, HostTarget& target);
    void removeEigenDHost(const String& serviceName);
    void detectRemovedEigenDHosts();

    void setMessageListener(MessageListener *m);
    void clearMessageListener(MessageListener *m);

    unsigned getNumHosts();
    HostTarget getHost(unsigned i);
    void hostListChanged();
    

    //--------------------------------------------------------------------------
    // ListBoxModel virtual functions
    

private:
    // the xmlrpc manager
    XMLRPCManager* xmlrpcManager_;
    
    // OSC rendezvous server
    lo_server rendezvousServer_;
    int serverFileDescriptor_;

    // map of eigenstage services and their host name and port number
    map<String, HostTarget> eigenDHosts_;
    
    // critical section to share the host list with the message thread safely
    CriticalSection hostListCriticalSection_;
    MessageListener* listener_;
};



//------------------------------------------------------------------------------
//
// XMLRPC Abort Exception
//
// indicates that should abort current XMLRPC execution because of failure
//
// used when shutting down app to indicate that current XMLRPC execution should abort
//
// also used by iPod build to abort getting tab updates when connections is lost
// without trying to fully halt and reconnect
//
//------------------------------------------------------------------------------

class XMLRPCAbortException
{
public:
    XMLRPCAbortException() {}
};

//------------------------------------------------------------------------------
//
// XMLRPC Manager
//
// manages the execution of XMLRPC calls to the eigenD XMLRPC server
//------------------------------------------------------------------------------

class XMLRPCManager: public Timer, public MessageListener
{
public:
    XMLRPCManager(MainComponent* mainComponent);
    ~XMLRPCManager();

    void initialize();
    void shutdownThreads();
    
    void suspend();
    void resume();

    bool connect(String &hostName, unsigned port);
    void browseForEigenD();
    void shutdown();
    void disableBrowser(bool b);
	void handleMessage (const Message& message);
    void timerCallback();
    void setAgentViewUpdate(bool agentViewUpdate);

    bool execute(const char *method, XmlRpcValue const &params, XmlRpcValue &result, bool retry);
    
    friend class ConnectionManager;

    //--------------------------------------------------------------------------
    // String conversions to interface Juce to the XMLRPC++ library
    
    static String stdToJuceString(const std::string& stdString);
    static std::string juceToStdString(const String& juceString);

private:

    MainComponent* mainComponent_;
    
    // client instance for the main thread
    XmlRpcClient* client_;

    ConnectionManager* connection_;
    int status_;
    String desired_host_;
    bool browser_disabled_;
    bool agentViewUpdate_;
    
    StatusDialogComponent* volatile statusDialog_;
};


//------------------------------------------------------------------------------
//
// OSC Manager
//
//------------------------------------------------------------------------------

class OSCManager : public Thread, public Timer
{
public:
    OSCManager(XMLRPCManager* xmlrpcManager);
    ~OSCManager();

    bool initialize();
    String getHostName();
    void shutdown();
    void discover();
    bool connect(const String &host,const char* port);
    void timerCallback();
    
    void sendFloat(String path, float value) const;
    void sendBlob(String path, const void *value, unsigned size) const;
    void sendString(String path, const String &value) const;
    void sendValue(String path, const WidgetData &value) const;
    
    int genericHandler(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg);
    static int staticGenericHandler(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data);
    static void staticErrorHandler(int num, const char *msg, const char *path);

    void addWidgetToPath(const String& path, WidgetComponent* widget);
    void removeWidgetFromPath(const String& path, WidgetComponent* widget);
    void requestAllWidgetValues();
    void requestWidgetValue(const String& path);

private:

    void run();

    //--------------------------------------------------------------------------

    XMLRPCManager* xmlrpcManager_;
    
    lo_address eigenDAddr_;
    lo_server server_;
    int serverFileDescriptor_;

    map< const String, set<WidgetComponent*>* > widgetMap_;

    CriticalSection widgetMapCriticalSection_;

};

//------------------------------------------------------------------------------
//
// Widget Value Message
//
// message passed from the OSC server thread to the widgets to update their
// value or set the latent value echoed from eigenD
//
//------------------------------------------------------------------------------

class WidgetConnectedMessage : public Message
{
    public:
        WidgetConnectedMessage(bool isConnected): Message(), isConnected_(isConnected) {}
    
        bool isConnected_;
};

class WidgetData
{
    public:
        WidgetData(): type_('f'), float_(0.0) {}
        WidgetData(const float value): type_('f'), float_(value) {}
        WidgetData(const void *value, unsigned size): type_('b'), blob_(value,size) {}
        WidgetData(const char *value): type_('s'), string_(String::fromUTF8(value)) {}
        WidgetData(const String &value): type_('s'), string_(value) {}

        bool operator!=(const WidgetData &other) const
        {
            return !(operator==(other));
        }

        bool operator==(const WidgetData &other) const
        {
            if(type_ != other.type_)
            {
                return false;
            }

            if(isFloat())
            {
                return float_ == other.float_;
            }

            if(isString())
            {
                return string_ == other.string_;
            }

            if(isBlob())
            {
                return blob_ == other.blob_;
            }

            return false;
        }

        bool isFloat() const { return type_=='f'; }
        bool isBlob() const { return type_=='b'; }
        bool isString() const { return type_=='s'; }

        char type_; // osc type code
        float float_;
        MemoryBlock blob_;
        String string_;
};

class WidgetValueMessage : public Message
{
    public:
        WidgetValueMessage(const float value): Message(), data(value) {}
        WidgetValueMessage(const void *value, unsigned size): Message(), data(value,size) {}
        WidgetValueMessage(const char *value): Message(), data(value) {}

        WidgetData data;
};

#endif // __NETWORK_H__






