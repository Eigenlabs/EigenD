
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

#include <picross/pic_resources.h>
#include <picross/pic_log.h>

int main(int ac, char **av)
{
    pic::logmsg() << "python prefix: " << pic::python_prefix_dir();
    pic::logmsg() << "global resources: " << pic::global_resource_dir();
    pic::logmsg() << "release resources: " << pic::release_resource_dir();
    pic::logmsg() << "global library: " << pic::global_library_dir();
    pic::logmsg() << "release library: " << pic::release_library_dir();
    pic::logmsg() << "public tools: " << pic::public_tools_dir();
    pic::logmsg() << "private tools: " << pic::private_tools_dir();
    pic::logmsg() << "release root: " << pic::release_root_dir();
    return 0;
}
