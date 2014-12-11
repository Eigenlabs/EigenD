/*
	oscpack -- Open Sound Control packet manipulation library
	http://www.audiomulch.com/~rossb/oscpack
	
	Copyright (c) 2004 Ross Bencina <rossb@audiomulch.com>
	
	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files
	(the "Software"), to deal in the Software without restriction,
	including without limitation the rights to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software,
	and to permit persons to whom the Software is furnished to do so,
	subject to the following conditions:
	
	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.
	
	Any person wishing to distribute modifications to the Software is
	requested to send the modifications to the original developer so that
	they can be incorporated into the canonical version.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
	ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
    OscDump prints incoming Osc packets. Unlike the Berkeley dumposc program
    OscDump uses a different printing format which indicates the type of each
    message argument.
*/


#include <iostream>
#include <sstream>
#include <map>
#include <functional>
#include <string>
#include<limits>
using std::map;
using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::numeric_limits;
using std::streamsize;
using std::istringstream;

#include "NetworkingUtils.h"
#include "UdpSocket.h"
#include "OscOutboundPacketStream.h"
#include "OscUdpZeroConfService.h"
#include "OscUdpZeroConfBrowser.h"
#include "OscUdpZeroConfResolver.h"

bool IsInt(const string& S)
{
   unsigned int n = S.size();
   unsigned int i = S.find_first_not_of(" \t");
   if (i == string::npos) return false;         // do not accept blanks
   if (S[i] == '-' || S[i] == '+')              // sign
   {
      i++;
      if (i >= n || S[i] == ' ' || S[i] == '\t')
         return false;                          // nothing following sign
   }
   for (unsigned int i1 = i; i1 < n; ++i1)
   {
      if (S[i1] < '0' ||  S[i1] > '9')
      {
         unsigned int i2 = S.find_first_not_of(" \t", i1);
         return i2 == string::npos;            // trailing blanks?
      }
   }
   return true;
}

// also include E format
bool IsFloat(const string& S)
{
   unsigned int n = S.size();
   bool dp = false;                             // have decimal point
   unsigned int i = S.find_first_not_of(" \t");
   if (i == string::npos) return false;         // do not accept blanks
   if (S[i] == '-' || S[i] == '+')              // sign
   {
      i++;
      if (i >= n || S[i] == ' ' || S[i] == '\t')
         return false;                          // nothing following sign
   }
   bool nos = false;
   for (unsigned int i1 = i; i1 < n; i1++)
   {
      if (S[i1] < '0' || S[i1] > '9')
      {
         if (S[i1] == '.')
         {
            if (dp) return false;               // two decimals
            dp = true;
         }
         else if (S[i1] == 'E' || S[i1] == 'e')
         {
            ++i1;
            if (i1 >=n) return false;           // nothing following E
            if (S[i1] == ' ' || S[i1] == '\t') return false;
            return IsInt(S.substr(i1));
         }
         else
         {
            unsigned int i2 = S.find_first_not_of(" \t", i1);
            return i2 == string::npos;          // trailing blanks?
         }
      }
      else nos = true;                          // at least one digit
   }
   return nos;
}


class App : public OSCRegisterListener
{
    typedef void (App::*command)(void);
    map<string,command> commands;
    bool quit;

    UdpTransmitSocket  *transmitSocket;

    OscUdpZeroConfService *service;

    void Browse()
    {
        cout << "Browse" << endl;

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(),'\n');    
    }
    void Send()
    {
        const long IP_MTU_SIZE  = 1536;
        char buffer[IP_MTU_SIZE];
        osc::OutboundPacketStream p( buffer, IP_MTU_SIZE );
        p.Clear();

        string line;
        cin.ignore(1);
        getline(cin,line);
        istringstream in(line);
        string address;
        in >> address;

        if(address.empty()) 
        {
            cout << "you need to specify and OSC address" << endl;
            return;
        }
        p << osc::BeginMessage( address.c_str() );

        while(in.good())
        {
            string str;
            in >> str;

            if(IsInt(str))
            {
                int v = atoi(str.c_str());
                p << v;
            }
            else if(IsFloat(str))
            {
                float v = atof(str.c_str());
                p << v;
            }
            else if(str == "false" )
                p << false;
            else if(str == "true" )
                p << true;
            else
                p << str.c_str();
        }

        p << osc::EndMessage;
    
        if(transmitSocket)
            transmitSocket->Send( p.Data(), p.Size() );
        else
            cout << "Not Connected to an OSC server" << endl;
    }

    void Connect()
    {
        string host;
        long port;
        cin >> host;
        cin >> port;

        if(cin.fail())
        {
            cout << "bad arguments" << endl;
            return;
        }

        if(transmitSocket)
            delete transmitSocket;
        transmitSocket = NULL;

        transmitSocket = new UdpTransmitSocket( IpEndpointName(host.c_str(),port) );

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(),'\n');    
    }
    void DisConnect()
    {
        if(transmitSocket)
            delete transmitSocket;
        transmitSocket = NULL;

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(),'\n');    
    }
    void Exit()
    {
        quit = true;

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(),'\n');    
    }
    void Register()
    {
        string name("");
        int port=0;
        cin >> name;
        cin >> port;

        if(service)
            delete service;
        service = new OscUdpZeroConfService(name.c_str(),port,this);
    }
    void Help()
    {
        cout << "available commands are:" << endl;
        for(map<string,command>::iterator i=commands.begin();i!=commands.end();++i)
        {
            cout << i->first << endl;
        }
    }
public:
    App()
        :transmitSocket(NULL)
        ,service(NULL)
        ,quit(false)
    {
        commands["browse"]      =   &App::Browse;
        commands["send"]        =   &App::Send;
        commands["connect"]     =   &App::Connect;
        commands["disconnect"]  =   &App::DisConnect;
        commands["exit"]        =   &App::Exit;
        commands["register"]    =   &App::Register;
        commands["help"]        =   &App::Help;
    }
    ~App()
    {
        if(transmitSocket)
            delete transmitSocket;

        if(service)
            delete service;
    }
    int Run()
    {
        //InitializeNetworking();

        while(!quit)
        {
            cout << "osc>";
            string cmd;
            cin >> cmd;

            if(cin.fail())
            {
                cout << "wrong arguments" << endl;
                continue;
            }

            if(!commands.count(cmd))
            {
                cout << "wrong command" << endl;
                continue;
            }
            else
            {
                (this->*commands[cmd]) ();
            }
        }

        //int port = 7000;
        //const char *destip = "localhost";
        //delete transmitPort;

        //TerminateNetworking();

        return 0;
    }
	virtual void OnRegisterService(const char *name)
    {

    }
};

int main(int argc, char* argv[])
{   
    App app;

    return app.Run();
}


