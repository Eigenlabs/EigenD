/*
 Copyright 2012 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#ifndef __CONDUCTOR_WIDGET_H__
#define __CONDUCTOR_WIDGET_H__

#include <piw/piw_data.h>

namespace cdtr
{
    class widget_provider_t;

    class widget_t
    {
        public:
            widget_t(const piw::change_t &receiver);
            ~widget_t();

            void change_value(const piw::data_t &value);
            bool invoke_rpc(int method, const piw::data_t &args, const piw::change_t &result);

            class impl_t;

        private:
            friend class widget_provider_t;
            impl_t *impl_;
    };

    class widget_provider_t
    {
        public:
            widget_provider_t();
            ~widget_provider_t();

            void initialise_widget(widget_t *);

            void change_value(const piw::data_t &value);
            virtual void value_changed(const piw::data_t &value) {}
            virtual bool rpc_invoked(void *context, int method, const piw::data_t &arguments) { return false; }
            void complete_rpc_string(void *context, const char *result);

            class impl_t;
        private:
            impl_t *impl_;
    };
};

#endif
