/*
  This file is part of Oscbonjour. 
	Copyright (c) 2005-2009 Rémy Muller. 
	
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

#include "ext.h"

#include "zeroconf/NetService.h"
#include "zeroconf/NetServiceBrowser.h"

using namespace ZeroConf;

#include <vector>
#include <string>
#include <algorithm>

#define VERSION "0.2"

class Oscbonjour;

//------------------------------------------------------------------------------------------------------------
//    class
//------------------------------------------------------------------------------------------------------------
typedef struct
{
  t_object o;
  Oscbonjour *oscbonjour;
  void *out0,*out1,*out2;
} oscbonjour_t;

static t_messlist *oscbonjour_class = NULL;


//------------------------------------------------------------------------------------------------------------
class Oscbonjour : public NetServiceListener, public NetServiceBrowserListener
{
  oscbonjour_t *external;
  NetServiceBrowser *browser;
  NetService *resolver;
  NetService *service;
  std::vector<std::string> services;
  typedef std::vector<std::string>::iterator veciterator;

public:
  Oscbonjour(oscbonjour_t *external)
    :external(external)
    ,browser(0)
    ,resolver(0)
    ,service(0)
  {
  }
  ~Oscbonjour()
  {
    if(browser) delete browser;
    if(resolver)delete resolver;
    if(service) delete service;
  }
  void Browse(const char *type, const char *domain)
  {
    services.clear();
    if(browser) delete browser;
    browser = 0;
    browser = new NetServiceBrowser();
    browser->setListener(this);
    browser->searchForServicesOfType(type, domain);
  }
  void Resolve(const char *name,const char *type,const char *domain)
  {
    if(resolver) delete resolver;
    resolver = 0;
    resolver = new NetService(domain,type,name);
    resolver->setListener(this);
    resolver->resolveWithTimeout(10.0);
  }
  void Register(const char *name,int port)
  {
    if(service) delete service;
    service = 0;
    service = new NetService("local.", "_osc._udp", name, port);
    service->setListener(this);
  }

  virtual void didFindService(NetServiceBrowser* pNetServiceBrowser, NetService *pNetService, bool moreServicesComing)
  {
    veciterator it = std::find(services.begin(),services.end(), pNetService->getName());
    if(it!=services.end()) return; // we already have it
    services.push_back(pNetService->getName());

    Atom at[1];
    SETSYM(at,gensym(const_cast<char*>(pNetService->getName().c_str())));

    if(external)
      outlet_anything(external->out2,gensym("append"),1,at);
  }

  virtual void didRemoveService(NetServiceBrowser *pNetServiceBrowser, NetService *pNetService, bool moreServicesComing)
  {
    veciterator it = std::find(services.begin(),services.end(), pNetService->getName());
    if(it==services.end()) return;      // we don't have it
    long index = it-services.begin();   // store the position
    services.erase(it);

    Atom at[1];
    SETLONG(at,index);

    if(external)
      outlet_anything(external->out2,gensym("delete"),1,at);
  }

  virtual void didResolveAddress(NetService *pNetService)
  {
    if(external)
    {
      Atom at[1];

      SETSYM(at,gensym(const_cast<char*>(pNetService->getHostName().c_str())));
      outlet_anything(external->out0,gensym("host"),1,at);

      SETLONG(at, pNetService->getPort());
      outlet_anything(external->out0,gensym("port"),1,at);
    }
  }

  virtual void didPublish(NetService *pNetService)
  {
    Atom at[1];
    SETSYM(at,gensym(const_cast<char*>(pNetService->getName().c_str())));

    if(external)
      outlet_anything(external->out1,gensym("realname"),1,at);
  }

private:
		virtual void willPublish(NetService *pNetService) {}
		virtual void didNotPublish(NetService *pNetService) {}
		virtual void willResolve(NetService *pNetService) {}
		virtual void didNotResolve(NetService *NetService) {}
		virtual void didUpdateTXTRecordData(NetService *pNetService) {}		
		virtual void didStop(NetService *pNetService) {}
		virtual void didFindDomain(NetServiceBrowser *pNetServiceBrowser, const std::string &domainName, bool moreDomainsComing) {}
		virtual void didRemoveDomain(NetServiceBrowser *pNetServiceBrowser, const std::string &domainName, bool moreDomainsComing) {}		
		virtual void willSearch(NetServiceBrowser *pNetServiceBrowser) {}
		virtual void didNotSearch(NetServiceBrowser *pNetServiceBrowser) {}
		virtual void didStopSearch(NetServiceBrowser *pNetServiceBrowser) {}
};

//------------------------------------------------------------------------------------------------------------
static void oscbonjour_version(oscbonjour_t *self, Symbol *s, short ac, Atom *at)
{
  post("oscbonjour (mDNS for maxmsp) version %s", VERSION);
}
//------------------------------------------------------------------------------------------------------------
//  user methods
//------------------------------------------------------------------------------------------------------------
static void oscbonjour_register(oscbonjour_t *self, Symbol *s, short ac, Atom *at)
{
  if(ac<2) return;
  if(at[0].a_type != A_SYM || at[1].a_type != A_LONG) return;

  int port    =   at[1].a_w.w_long; 
  self->oscbonjour->Register(at[0].a_w.w_sym->s_name,port);
}
//------------------------------------------------------------------------------------------------------------
static void oscbonjour_browse(oscbonjour_t *self, Symbol *s, short ac, Atom *at)
{
  outlet_anything(self->out2,gensym("clear"),0,NULL);

  const char *type      =   "_osc._udp";
  const char *domain    =   "local.";

  self->oscbonjour->Browse(type,domain);
}
//------------------------------------------------------------------------------------------------------------
static void oscbonjour_resolve(oscbonjour_t *self, Symbol *s, short ac, Atom *at)
{
  if(ac<1) return;
  if(at[0].a_type != A_SYM) return;

  const char *name      =   at[0].a_w.w_sym->s_name;
  const char *type      =   "_osc._udp";
  const char *domain    =   "local.";

  self->oscbonjour->Resolve(name,type,domain); 
}
//------------------------------------------------------------------------------------------------------------
static void oscbonjour_assist(oscbonjour_t *x, void *b, long msg, long a, char *dst)
{
  if (msg == 1) //inlet
  {
    sprintf(dst,"messages: browse, resolve, register");
  } 
  else if (msg == 2)  //outlet
  {
    switch(a)
    {
    case 0: sprintf(dst,"resolved host and port for service name");     break;
    case 1: sprintf(dst,"real registered service name");                break;
    case 2: sprintf(dst,"available services, connect to menu");             break;
    default: break;
    }
  }
}
//------------------------------------------------------------------------------------------------------------
static void *oscbonjour_new(Symbol *s, short ac, Atom *at)
{
  oscbonjour_t *self = (oscbonjour_t *)newobject(oscbonjour_class);

  self->oscbonjour = new Oscbonjour(self);
  self->out2 = outlet_new(self,0L);
  self->out1 = outlet_new(self,0L);
  self->out0 = outlet_new(self,0L);

  if(ac>0)
  {
    if(at[0].a_type != A_SYM) 
    {
      post("bad arguments");
      return self;
    }
    Symbol *sym = at[0].a_w.w_sym;
    std::string cmd(sym->s_name);

    at++;
    ac--;

    if      (cmd == "register") oscbonjour_register (self,sym,ac,at);
    else if (cmd == "browse")   oscbonjour_browse   (self,sym,ac,at);
    else if (cmd == "resolve")  oscbonjour_resolve  (self,sym,ac,at);
  }

  return self;
}
//------------------------------------------------------------------------------------------------------------
static void oscbonjour_free(oscbonjour_t *self)
{
  if(self->oscbonjour) delete self->oscbonjour;
}

//------------------------------------------------------------------------------------------------------------
// Entry Point
//------------------------------------------------------------------------------------------------------------
int main(void)
{
  setup(&oscbonjour_class, (method)oscbonjour_new, (method)oscbonjour_free, (short)sizeof(oscbonjour_t), 0L, A_GIMME, 0);

  addmess((method)oscbonjour_register, "register", A_GIMME, 0);
  addmess((method)oscbonjour_browse,   "browse",   A_GIMME, 0);
  addmess((method)oscbonjour_resolve,  "resolve",  A_GIMME, 0);

  addmess((method)oscbonjour_assist,   "assist",   A_CANT, 0);

  oscbonjour_version(0, 0, 0, 0);
}
