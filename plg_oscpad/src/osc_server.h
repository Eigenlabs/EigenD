
#ifndef __OSC_SERVER__
#define __OSC_SERVER__

#include <piw/piw_tsd.h>
#include <stack>
#include <lib_lo/lo/lo.h>


namespace oscpad_plg
{
	struct osc_handler_t
	{
	    virtual void shutdown() {}
		virtual const char *topic() = 0;
	    virtual void messageHandler(const char *path,lo_message& msg)=0;
        virtual ~osc_handler_t() {} ;
	};


	// a simple handler which logs messages
	struct osc_log_handler_t : osc_handler_t
	{
		virtual const char *topic();
	    virtual void messageHandler(const char *path,lo_message& msg);
        virtual ~osc_log_handler_t() {} ;
	};


    class osc_server_t: public pic::thread_t
    {
        public:
            osc_server_t(const std::string &port);
            virtual ~osc_server_t();

            // thread functions
            void thread_main();
            void thread_init();
            void thread_term();

            // start and stop the server
            void osc_startup();
            void osc_shutdown();

            // for listening to incoming messages, reception is on the listening thread
            void registerHandler(osc_handler_t *handler);

        private:
            bool stop_;
            lo_server client_;
            std::string port_;
            std::stack<osc_handler_t*> handlers_;
    };
};

#endif
