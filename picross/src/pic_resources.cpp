
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

#include <picross/pic_config.h>

#ifdef PI_WINDOWS
#define UNICODE 1
#define _UNICODE 1
#define RES_SEPERATOR '\\'
#define RES_SEPERATOR_STR "\\"
#define RES_PATH_MAX MAX_PATH
#include <windows.h>
#include <shlobj.h>
#else
#include <dlfcn.h>
#include <limits.h>
#include <stdlib.h>
#define RES_SEPERATOR '/'
#define RES_SEPERATOR_STR "/"
#define RES_PATH_MAX PATH_MAX
#endif

#include <picross/pic_resources.h>
#include <string.h>

namespace
{
    static bool is_debug();

    static void dirname(char *buffer)
    {
        char *p = strrchr(buffer,RES_SEPERATOR);

        if(p)
        {
            *p = 0;
        }
        else
        {
            buffer[0] = RES_SEPERATOR;
            buffer[1] = 0;
        }
    }

#ifdef PI_WINDOWS
    static void get_exe(char *buffer)
    {
        WCHAR dest [RES_PATH_MAX+1];
        HINSTANCE moduleHandle = GetModuleHandle(0);
        GetModuleFileName (moduleHandle, dest, RES_PATH_MAX+1);
        WideCharToMultiByte(CP_UTF8,0,dest,-1,buffer,RES_PATH_MAX+1,0,0);
    }

    static void get_prefix(char *buffer)
    {
        if(is_debug())
        {
            WCHAR dest [RES_PATH_MAX+1];
            SHGetSpecialFolderPath (0, dest, CSIDL_PROGRAM_FILES, 0);
            WideCharToMultiByte(CP_UTF8,0,dest,-1,buffer,RES_PATH_MAX+1,0,0);
            strcat(buffer,RES_SEPERATOR_STR);
            strcat(buffer,"Eigenlabs");
        }
        else
        {
            get_exe(buffer); // .../release-1.0/bin/xx.exe
            dirname(buffer); // .../release-1.0/bin
            dirname(buffer); // .../release-1.0
            dirname(buffer); // ...
        }
    }

    static void get_lib(char *buffer)
    {
        WCHAR dest [RES_PATH_MAX+1];
        SHGetSpecialFolderPath (0, dest, CSIDL_PERSONAL, 0);
        WideCharToMultiByte(CP_UTF8,0,dest,-1,buffer,RES_PATH_MAX+1,0,0);
        strcat(buffer,RES_SEPERATOR_STR);
        strcat(buffer,"Eigenlabs");
    }

    static void get_pyprefix(char *buffer)
    {
        get_prefix(buffer);
        strcat(buffer,RES_SEPERATOR_STR);
        strcat(buffer,"runtime-1.0.0");
        strcat(buffer,RES_SEPERATOR_STR);
        strcat(buffer,"Python26");
    }

    static void get_pubtool(char *buffer)
    {
        get_exe(buffer);
        dirname(buffer);
    }

    static void get_pritool(char *buffer)
    {
        get_exe(buffer);
        dirname(buffer);
    }

#endif

#ifdef PI_MACOSX
    static void get_exe(char *buffer)
    {
        Dl_info exeInfo;
        dladdr ((const void*) get_exe, &exeInfo);
        realpath(exeInfo.dli_fname,buffer);
    }

    static void get_prefix(char *buffer)
    {
        if(is_debug())
        {
            strcpy(buffer,"/usr/pi");
        }
        else
        {
            get_exe(buffer); // .../release-1.0/bin/xx.exe
            dirname(buffer); // .../release-1.0/bin
            dirname(buffer); // .../release-1.0
            dirname(buffer); // ...
        }
    }

    static void get_lib(char *buffer)
    {
        strcpy(buffer,getenv("HOME"));
        strcat(buffer,"/Library/Eigenlabs");
    }

    static void get_pyprefix(char *buffer)
    {
        strcpy(buffer,PI_PREFIX);
    }

    static void get_pubtool(char *buffer)
    {
        if(!is_debug())
        {
            strcpy(buffer,"/Applications/Eigenlabs/");
            strcat(buffer,PI_RELEASE);
        }
        else
        {
            get_exe(buffer);
            dirname(buffer);
            dirname(buffer);
            strcat(buffer,RES_SEPERATOR_STR);
            strcat(buffer,"app");
        }
    }

    static void get_pritool(char *buffer)
    {
        if(is_debug())
        {
            get_exe(buffer);
            dirname(buffer);
            dirname(buffer);
            strcat(buffer,RES_SEPERATOR_STR);
            strcat(buffer,"app");
        }
        else
        {
            get_exe(buffer);
            dirname(buffer);
            dirname(buffer);
            strcat(buffer,RES_SEPERATOR_STR);
            strcat(buffer,"Applications");
        }
    }


#endif

#ifdef PI_LINUX
    static void get_exe(char *buffer)
    {
        Dl_info exeInfo;
        dladdr ((const void*) get_exe, &exeInfo);
        if(!realpath(exeInfo.dli_fname,buffer)) buffer[0]=0;
    }

    static void get_lib(char *buffer)
    {
        strcpy(buffer,getenv("HOME"));
        strcat(buffer,"/.Eigenlabs");
    }

    static void get_prefix(char *buffer)
    {
        if(is_debug())
        {
            strcpy(buffer,"/usr/pi");
        }
        else
        {
            get_exe(buffer); // .../release-1.0/bin/xx.exe
            dirname(buffer); // .../release-1.0/bin
            dirname(buffer); // .../release-1.0
            dirname(buffer); // ...
        }
    }

    static void get_pyprefix(char *buffer)
    {
        strcpy(buffer,"/usr");
    }

    static void get_pubtool(char *buffer)
    {
        get_exe(buffer);
        dirname(buffer);
    }

    static void get_pritool(char *buffer)
    {
        get_exe(buffer);
        dirname(buffer);
    }

#endif
    static int __is_debug = 0;

    bool is_debug()
    {
        char buffer[RES_PATH_MAX+1];

        if(__is_debug != 0)
        {
            return (__is_debug>0);
        }

        get_exe(buffer); // .../release-1.0/bin/xx.exe
        dirname(buffer); // .../release-1.0/bin
        dirname(buffer); // .../release-1.0

        char *p = strrchr(buffer,RES_SEPERATOR);
        
        if(!p)
        {
            __is_debug=1;
            return true;
        }

        if(p[0] && !strcmp(&p[1],"tmp"))
        {
            __is_debug=1;
            return true;
        }

        __is_debug=-1;
        return false;
    }
};

char pic::platform_seperator()
{
    return RES_SEPERATOR;
}

std::string pic::global_resource_dir()
{
    char buffer[RES_PATH_MAX+1];
    get_prefix(buffer);
    return buffer;
}

std::string pic::python_prefix_dir()
{
    char buffer[RES_PATH_MAX+1];
    get_pyprefix(buffer);
    return buffer;
}

std::string pic::release_root_dir()
{
    char buffer[RES_PATH_MAX+1];
    get_exe(buffer);
    dirname(buffer);
    dirname(buffer);
    return buffer;
}

std::string pic::release_resource_dir()
{
    char buffer[RES_PATH_MAX+1];
    get_exe(buffer);
    dirname(buffer);
    dirname(buffer);
    strcat(buffer,RES_SEPERATOR_STR);
    strcat(buffer,"resources");
    return buffer;
}

std::string pic::global_library_dir()
{
    char buffer[RES_PATH_MAX+1];
    get_lib(buffer);
    return buffer;
}


std::string pic::release_library_dir()
{
    char buffer[RES_PATH_MAX+1];
    get_lib(buffer);
    strcat(buffer,RES_SEPERATOR_STR);
    strcat(buffer,PI_RELEASE);
    return buffer;
}

std::string pic::public_tools_dir()
{
    char buffer[RES_PATH_MAX+1];
    get_pubtool(buffer);
    return buffer;
}

std::string pic::private_tools_dir()
{
    char buffer[RES_PATH_MAX+1];
    get_pritool(buffer);
    return buffer;
}

std::string pic::release()
{
    return PI_RELEASE;
}

std::string pic::username()
{
#ifdef PI_WINDOWS
    return getenv("USERNAME");
#else
    return getenv("USER");
#endif
}
