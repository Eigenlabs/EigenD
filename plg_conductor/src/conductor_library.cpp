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

#include "conductor_library.h"

#include <picross/pic_error.h>


cdtr::conductor_library_t::conductor_library_t(const std::string &library_path): library_path_(library_path)
{
}

const std::string &cdtr::conductor_library_t::get_library_dir()
{
    return library_path_;
}

juce::File cdtr::conductor_library_t::get_conductor_dir(const char *name)
{
    juce::File dir(juce::String(get_library_dir().c_str()));
    dir = dir.getChildFile(name);

    bool dir_ok = false;
    if(!dir.exists())
    {
        dir_ok = dir.createDirectory().wasOk();
    }
    else
    {
        dir_ok = dir.isDirectory();
    }

    if(dir_ok)
    {
        dir_ok = dir.hasWriteAccess();
    }

    if(!dir_ok)
    {
        PIC_THROW((juce::String("Unable to get the conductor directory: ")+dir.getFullPathName()).toUTF8());
    }

    return dir;
}

juce::File cdtr::conductor_library_t::get_conductor_file(const char *name)
{
    juce::File file(juce::String(get_library_dir().c_str()));
    file = file.getChildFile(name);

    bool file_ok = false;
    if(file.exists())
    {
        file_ok = !file.isDirectory();
    }
    else
    {
        file_ok = true;
    }

    if(file_ok)
    {
        file_ok = file.hasWriteAccess();
    }

    if(!file_ok)
    {
        PIC_THROW((juce::String("Unable to get the conductor file: ")+file.getFullPathName()).toUTF8());
    }

    return file;
}
