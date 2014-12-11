#include "osc_server.h"

#include <map>

static void errorHandler_(int num, const char *msg, const char *where)
{
	const char* w=where;
	const char* m=msg;
	if(!w) w="Null";
	if(!m) m="Null";

	pic::logmsg() << "liblo server error " << num << " in path: " << w << "msg:" <<  m;
}

static int messageHandler_(const char *path, const char *types, lo_arg ** argv, int argc, lo_message msg, void *user_data)
{
//	pic::logmsg() << "OSC message RAW: " << path  << "addr:" << (long) user_data;
	((livepad_plg::osc_handler_t*) user_data)->messageHandler(path,msg);
	return 1;
}



namespace livepad_plg
{

const char *osc_log_handler_t::topic()
{
	return NULL; //represents ALL topics
}
void osc_log_handler_t::messageHandler(const char *path,lo_message& msg)
{
	 pic::logmsg() << "osc_log_handler_t::messageHandler: " << path;
}


osc_server_t::osc_server_t(const std::string &port)
	: stop_(false), port_(port)
{
}

osc_server_t::~osc_server_t()
{
    osc_shutdown();
}


void osc_server_t::osc_startup()
{
    osc_shutdown();
    stop_ = false;
    run();
}

void osc_server_t::osc_shutdown()
{
	if(!stop_)
	{
		stop_=true;
		// pic::logmsg() << "osc_server_t::osc_shutdown";
		wait();
	}
}


void osc_server_t::thread_init()
{
	// pic::logmsg() << "create server on " << port_ ;
    client_ = lo_server_new(port_.c_str(), errorHandler_);
    if (client_ == NULL)
    {
		pic::logmsg() << "failed to create server" << port_ ;
		stop_=true;
		return;
    }
//    registerHandler(new osc_log_handler_t());
}

void osc_server_t::thread_term()
{
	// pic::logmsg() << "osc_server_t::thread_term";
	osc_handler_t *handler;
	while(!handlers_.empty())
	{
		handler = handlers_.top();
		if(client_) lo_server_del_method(client_,handler->topic(),NULL,messageHandler_,(void*) handler);
		handlers_.pop();
		handler->shutdown();
		delete handler;
	}
	// pic::logmsg() << "osc_server_t::thread_term, freeing server";
    lo_server_free(client_);
    client_=NULL;
}


void osc_server_t::thread_main()
{
    while (!stop_)
    {
        lo_server_recv_noblock(client_, 10);
    }
}

void osc_server_t::registerHandler(osc_handler_t *handler)
{
	//MSH change to ref_t for handler?
	// pic::logmsg() << "registerHandler" << handler->topic() ;
	if(client_) lo_server_add_method(client_,handler->topic(),NULL,messageHandler_,(void*) handler);
	handlers_.push(handler);
}



}; //oscpad_plg
