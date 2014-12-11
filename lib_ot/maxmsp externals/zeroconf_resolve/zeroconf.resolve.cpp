/**
 */

#include "ext.h"				
#include "ext_obex.h"						
#include "zeroconf/NetService.h"
#include "zeroconf/NetServiceThread.h"
#include <iostream>

using namespace ZeroConf;

class Service;

struct zeroconf_resolve 
{
  t_object  ob;			// the object itself (must be first)
  t_symbol *name;
  t_symbol *type;
  t_symbol *domain;
  void *out;
  Service *mpService;
};

class Service : public NetService, NetServiceListener
{
  zeroconf_resolve *mpExternal;
  
public:
  Service(const std::string &domain, 
                  const std::string &type, 
                  const std::string &name,
                  zeroconf_resolve *x)
  : NetService(domain, type, name)
  , mpExternal(x)
  {
		setListener(this); 
  }
  
  virtual void willPublish(NetService *pNetService) {}
  virtual void didNotPublish(NetService *pNetService) {}
  virtual void didPublish(NetService *pNetService) {}
  virtual void willResolve(NetService *pNetService) {}
  virtual void didNotResolve(NetService *NetService)
  {
    object_post((t_object *)mpExternal, "didNotResolve" );
  }
  virtual void didResolveAddress(NetService *pNetService)
  {
      t_atom at[1];
	  
      atom_setsym(at,gensym(const_cast<char*>(pNetService->getHostName().c_str())));
      outlet_anything(mpExternal->out, gensym("host"), 1, at);
	  
      atom_setlong(at, pNetService->getPort());
      outlet_anything(mpExternal->out, gensym("port"), 1, at);
  }
  virtual void didUpdateTXTRecordData(NetService *pNetService) {}   
  virtual void didStop(NetService *pNetService) {}
};

//------------------------------------------------------------------------------
t_class *zeroconf_resolve_class;

void zeroconf_resolve_poll(zeroconf_resolve *x, t_symbol *sym, short argc, t_atom *arv)
{	
	// poll for results
	if(x->mpService && x->mpService->getDNSServiceRef())
	{
		DNSServiceErrorType err = kDNSServiceErr_NoError;
		if(NetServiceThread::poll(x->mpService->getDNSServiceRef(), 0.001, err))
		{
			if(err > 0)
				object_post((t_object*)x, "error %d", err);
			x->mpService->stop();
		}
	}
	
	if(x->mpService && x->mpService->getDNSServiceRef()) // we check again, because it might have change in reaction to a callback
	{
		schedule_defer(x, (method)zeroconf_resolve_poll, 1000, NULL, 0, NULL); // reschedule in 1 sec
	}
}

void zeroconf_resolve_bang(zeroconf_resolve *x)
{
  if(x->mpService)
    delete x->mpService;
	x->mpService = NULL;
  
  x->mpService = new Service(x->domain->s_name,
                             x->type->s_name,
                             x->name->s_name,
                             x);
  x->mpService->resolveWithTimeout(10.0, false);
	
	schedule_defer(x, (method)zeroconf_resolve_poll, 1000, NULL, 0, NULL); // reschedule in 1 sec
}

void zeroconf_resolve_assist(zeroconf_resolve *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) 
  { 
		sprintf(s, "I am inlet %ld", a);
	} 
	else 
  {	
		sprintf(s, "I am outlet %ld", a); 			
	}
}

void zeroconf_resolve_free(zeroconf_resolve *x)
{
  if(x->mpService)
  {
    delete x->mpService;
  }
}

void *zeroconf_resolve_new(t_symbol *s, long argc, t_atom *argv)
{
	zeroconf_resolve *x = NULL;
  
	if (x = (zeroconf_resolve *)object_alloc(zeroconf_resolve_class)) 
	{
    x->mpService = NULL;
    x->name = gensym("");
    x->type = gensym("");
    x->domain = gensym("local.");
	 	x->out = outlet_new(x, NULL);
    attr_args_process(x, argc, argv);	
		
		zeroconf_resolve_bang(x);
	}
	return (x);
}

int main(void)
{		
	t_class *c = class_new("zeroconf.resolve", (method)zeroconf_resolve_new, (method)zeroconf_resolve_free, (long)sizeof(zeroconf_resolve), 0L, A_GIMME, 0);
	
  class_addmethod(c, (method)zeroconf_resolve_bang,			"bang",     0);  
  class_addmethod(c, (method)zeroconf_resolve_assist,		"assist",   A_CANT, 0);  

  CLASS_ATTR_SYM(c, "name", 0, zeroconf_resolve, name);
  CLASS_ATTR_SYM(c, "type", 0, zeroconf_resolve, type);
  CLASS_ATTR_SYM(c, "domain", 0, zeroconf_resolve, domain);
	
	class_register(CLASS_BOX, c); /* CLASS_NOBOX */
	zeroconf_resolve_class = c;
  
	return 0;
}
