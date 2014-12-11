#include "osc_client.h"

namespace livepad_plg
{

osc_client_t::osc_client_t(const std::string& host, const std::string &port)
	: client_(NULL), host_(host), port_(port)
{
}

osc_client_t::~osc_client_t()
{
    osc_shutdown();
}


void osc_client_t::osc_startup()
{
	//pic::logmsg() << "osc_client_t::osc_startup " << host_ << port_ ;
    client_ = lo_address_new(host_.c_str(),port_.c_str());
    if (client_ == NULL)
    {
		pic::logmsg() << "failed to create client" << port_ ;
		return;
    }
}

void osc_client_t::osc_shutdown()
{
	if(client_!=NULL)
		lo_address_free(client_);
	client_=NULL;
}




bool osc_client_t::send(osc_message_t *msg)
{
    // pic::logmsg() << "osc_client_t::send sending to:" << (long) client_ << " msg:" << msg->topic();
	if (client_ == NULL) return false;
	lo_message m=msg->createMessage();
	int ret = lo_send_message(client_, msg->topic(), m);
    if (ret == -1)
    {
	        pic::logmsg() << "An error occured:" <<  lo_address_errstr(client_);
	        return false;
    }
    lo_message_free(m);
    delete msg;
    return true;
}

}; //oscpad_plg
