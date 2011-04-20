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

#ifndef __PIW_ANCHOR__
#define __PIW_ANCHOR__
#include "piw_exports.h"
#include "piw_server.h"
#include "piw_tsd.h"
#include "piw_data.h"
#include <picross/pic_log.h>

namespace piw
{
    class  anchor_t
    {
        public:
            anchor_t(): address_("")
            {
            }

            void set_address_str(const std::string &a)
            {
                close();
                address_ = a;
                open();
            }

            void set_address(const piw::data_nb_t &a)
            {
                close();

                if(a.is_string())
                {
                    address_=a.as_string();
                }
                else
                {
                    address_="";
                }

                open();
            }

            void set_server(server_t *s)
            {
                close();
                _server=s;
                open();
            }

            ~anchor_t()
            {
                close();
            }

        private:
            void open()
            {
                if(_server.isvalid() && address_.length()>0)
                {
                    tsd_server(address_.c_str(),_server.ptr());
                }
            }

            void close()
            {
                if(_server.isvalid())
                {
                    _server->close_server();
                }
            }

        private:
            pic::weak_t<server_t> _server;
            std::string address_;
    };
}

#endif
