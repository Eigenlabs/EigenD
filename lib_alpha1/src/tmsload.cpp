
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

#include <lib_alpha1/alpha1_download.h>
#include <iostream>

struct download_t
{
    const char *filename;
    void operator()(const char *name) const { alpha1::tms_download(name,filename); }
    bool operator==(const download_t &) const { return true; }
};

void download_all(const char *filename)
{
    download_t d; d.filename=filename;
    pic::usbenumerator_t::enumerate(0x0451,0x9001,pic::f_string_t::callable(d));
}

int main(int ac, char **av)
{
    if(ac < 2)
    {
        std::cerr << "usage: tmsload firmware-file [device-name]\n";
        return -1;
    }

    const char *filename = av[1];

    try
    {
        if(av[2])
        {
            alpha1::tms_download(av[2],filename);
        }
        else
        {
            download_all(filename);
        }
    }
    catch(const std::exception &e)
    {
        std::cerr << e.what() << "\n";
        return -1;
    }
    catch(...)
    {
        std::cerr << "unknown exception\n";
        return -1;
    }

    return 0;
}
