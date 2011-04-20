
/*
 Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com

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

//------------------------------------------------------------------------------
// lng_bonjour.cpp: Bonjour Service Registration
//
// uses the DNS-SD API to advertise services
//------------------------------------------------------------------------------

#include "lng_bonjour.h"
#include <picross/pic_log.h>

#define USE_BONJOUR 0

#if USE_BONJOUR==1
// define this to stop dns_sd from trying to redefine basic types for windows
#ifdef _WIN32
#define _MSL_STDINT_H
#endif // _WIN32
#include <dns_sd.h>
#endif // USE_BONJOUR==1

namespace language 
{
    
    //--------------------------------------------------------------------------
    // Bonjour impl class
    //--------------------------------------------------------------------------
    
    struct bonjour_t::impl_t
    {
        impl_t(unsigned server_port);
        ~impl_t();
        
        
        void serviceRegister();
        
#if USE_BONJOUR==1
        static void staticServiceRegisterCallback(DNSServiceRef sdRef, 
                                                  DNSServiceFlags flags, 
                                                  DNSServiceErrorType errorCode, 
                                                  const char *name, 
                                                  const char *regtype, 
                                                  const char *domain, 
                                                  void *context );
#endif // USE_BONJOUR==1
        unsigned server_port_;

#if USE_BONJOUR==1
        DNSServiceRef serviceReference_;
#endif // USE_BONJOUR==1
        
    };
    
    //--------------------------------------------------------------------------
    // Bonjour impl class functions
    //--------------------------------------------------------------------------
    
    
    bonjour_t::impl_t::impl_t(unsigned server_port) : server_port_(server_port)
    {
#if USE_BONJOUR==1
        serviceReference_ = 0;
#endif // USE_BONJOUR==1
    }
    
    bonjour_t::impl_t::~impl_t()
    {
#if USE_BONJOUR==1
        if(serviceReference_)
            DNSServiceRefDeallocate(serviceReference_);
#endif // USE_BONJOUR==1
    }

#if USE_BONJOUR==1
    void bonjour_t::impl_t::staticServiceRegisterCallback(DNSServiceRef sdRef, 
                                       DNSServiceFlags flags, 
                                       DNSServiceErrorType errorCode, 
                                       const char *name, 
                                       const char *regtype, 
                                       const char *domain, 
                                       void *context )
    {
        
        // TODO: handle error
        if(errorCode==kDNSServiceErr_NoError)
            pic::logmsg() << "Bonjour name="<< name<<" type=" << regtype << " domain=" << domain;
        else
            pic::logmsg() << "Bonjour service register callback error code=" << errorCode;

    
    }
#endif // USE_BONJOUR==1

    
    void bonjour_t::impl_t::serviceRegister()
    {

#if USE_BONJOUR==1
        // register service for Stage XMLRPC server
        
        DNSServiceErrorType errorCode = DNSServiceRegister (&serviceReference_,
                                                0,                              // rename behaviour flags, NULL is default
                                                0,                              // register on all interfaces (ethernet, airport etc)
                                                0,                              // name, NULL = this computer
                                                "_eigenstage._tcp",             // service type for Stage XMLRPC server
                                                0,                              // domain
                                                0,                              // host
                                                htons(server_port_),            // port
                                                0,                              // length of text record
                                                0,                              // text record
                                                (DNSServiceRegisterReply)bonjour_t::impl_t::staticServiceRegisterCallback,  //
												this                            // app context pointer
                                                );  

        // TODO: handle error
        if (errorCode!=kDNSServiceErr_NoError) 
        {
            pic::logmsg() << "Bonjour service register error code=" << errorCode;
        }
#endif // USE_BONJOUR==1
        
    }
    
    
    
    
    
    
    
    
    
    


    //--------------------------------------------------------------------------
    // Bonjour impl interface
    //--------------------------------------------------------------------------

    bonjour_t::bonjour_t(unsigned server_port): impl_(new impl_t(server_port))
    {
    }

    bonjour_t::~bonjour_t()
    {
        delete impl_;
    }

    
    void bonjour_t::service_register()
    {
        impl_->serviceRegister();
    }


} // namespace language







