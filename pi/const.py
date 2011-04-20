
#
# Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com
#
# This file is part of EigenD.
#
# EigenD is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# EigenD is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
#

server_rtransient = 0x04
server_list = 0x10
server_transient = 0x20
server_ro = 0x40
server_fast = 0x80

client_sync = 1
client_clock = 2

fastdata_sender = 1

bctlink_maxpayload = (49152-48)
bctvtype_null = 0
bctvtype_path = 1
bctvtype_string = 2
bctvtype_array = 3
bctvtype_float = 4
bctvtype_int = 5
bctvtype_bool = 6
bctvtype_blob = 7

meta_node = 255
data_node = 254
ext_node = 253
verb_node = 252

meta_domain = 1
meta_master = 2
meta_protocols = 3
meta_adjectives = 4
meta_private = 6
meta_ordinal = 7
meta_names = 8
meta_latency = 9
meta_frelation = 10
meta_relation = 11
meta_fuzzy = 12
meta_insert = 14
meta_pronoun = 15
meta_icon = 16
meta_verb = 17
meta_mode = 18
meta_macro = 19
meta_ideals = 20
meta_control = 21
meta_notify = 22

light_off = 0
light_active = 1
light_inactive = 2
light_unknown = 3
light_mixed = 4

status_off = 0
status_active = 1
status_inactive = 2
status_unknown = 3
status_mixed = 4
status_blink = 5
status_selector_on = 6
status_selector_off = 7
status_choose_available = 8
status_choose_used = 9
status_choose_active = 10

scope_global = 0
scope_pernote = 1
scope_channel = 2

resolution_bits_7 = 7
resolution_bits_14 = 14
