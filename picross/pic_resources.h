
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

#ifndef __PIC_RESOURCES_H__
#define __PIC_RESOURCES_H__

#include "pic_exports.h"
#include <string>

namespace pic
{
    PIC_DECLSPEC_FUNC(std::string) global_resource_dir();
    PIC_DECLSPEC_FUNC(std::string) release_resource_dir();
    PIC_DECLSPEC_FUNC(std::string) release_root_dir();
    PIC_DECLSPEC_FUNC(std::string) global_library_dir();
    PIC_DECLSPEC_FUNC(std::string) release_library_dir();
    PIC_DECLSPEC_FUNC(std::string) python_prefix_dir();
    PIC_DECLSPEC_FUNC(std::string) public_tools_dir();
    PIC_DECLSPEC_FUNC(std::string) private_tools_dir();
    PIC_DECLSPEC_FUNC(char) platform_seperator();
    PIC_DECLSPEC_FUNC(std::string) release();
};

#endif
