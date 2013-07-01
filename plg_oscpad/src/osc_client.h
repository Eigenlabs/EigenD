
#ifndef __OSC_CLIENT__
#define __OSC_CLIENT__

#include <piw/piw_tsd.h>
#include <lib_lo/lo/lo.h>


namespace oscpad_plg
{

	struct osc_message_t
	{
		virtual const char *topic() = 0;
		virtual lo_message createMessage() = 0;
        virtual ~osc_message_t() {} ;
	};


    class osc_client_t: public pic::nocopy_t
    {
        public:
            osc_client_t(const std::string &host, const std::string &port);
            virtual ~osc_client_t();

            // start and stop the client
            void osc_startup();
            void osc_shutdown();

            // for sending messages, note this is done on what ever thread you call it on
            bool send(osc_message_t *msg);

        private:
            lo_address client_;
            std::string host_;
            std::string port_;
    };
};

#endif
