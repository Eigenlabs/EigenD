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

#ifndef __CDTR_CONDUCTOR_LIBRARY_H__
#define __CDTR_CONDUCTOR_LIBRARY_H__

#include <lib_juce/juce.h>

namespace cdtr
{
    class conductor_library_t
    {
        public:
            conductor_library_t(const std::string &library_path);
    
            const std::string &get_library_dir();

            juce::File get_conductor_dir(const char *name);
            juce::File get_conductor_file(const char *name);
    
        private:
            std::string library_path_;
    };
};

#endif
