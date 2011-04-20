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

#ifndef __PIW_AGENT__
#define __PIW_AGENT__

#include <pibelcanto/plugin.h>
#include <picross/pic_log.h>

#define PIW_DEFINE_AGENT(name,klass) \
 extern "C" const char *bct_agent_describe() { return name; } \
 extern "C" bct_server_t *bct_agent_create(bct_entity_t c, const char *p) { \
    piw::tsd_setcontext(c); try { return new klass(p); } CATCHLOG() return 0; \
 } \
 extern "C" void bct_agent_delete(bct_entity_t c,bct_server_t *agent) { \
    piw::tsd_setcontext(c); try { delete (klass *)(agent); } CATCHLOG() \
 }

#endif
