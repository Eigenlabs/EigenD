
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

#include <ApplicationServices/ApplicationServices.h>
#include <picross/pic_tool.h>
#include <string>

struct pic::tool_t::impl_t
{
    impl_t(const std::string &dir,const char *name): started_(false)
    {
        path_ = dir;
        path_ = path_+"/"+name+".app";
    }

    ~impl_t()
    {
        quit();
    }

    void quit()
    {
        if(started_)
        {
            started_=false;
            kill(pid_,9);

            while(kill(pid_,0)>=0)
            {
                sleep(1);
            }
        }
    }

    bool is_running()
    {
        if(started_)
        {
            if(kill(pid_,0)>=0)
            {
                return true;
            }

            started_=false;
        }

        return false;
    }

    void start()
    {
        OSStatus status = noErr;
        FSRef ref;

        status = FSPathMakeRef((const UInt8 *)path_.c_str(), &ref, NULL);

        if (status != noErr)
        {
            printf("can't create file ref %ld\n",(long)status);
            return;
        }

        LSApplicationParameters params = {0, kLSLaunchDefaults, &ref, NULL, NULL, NULL};
        status = LSOpenApplication(&params, &psn_);

        if (status != noErr)
        {
            printf("process not started: %ld\n",(long)status);
            return;
        }

        started_=true;
        GetProcessPID(&psn_,&pid_);
    }

    std::string path_;
    bool started_;
    ProcessSerialNumber psn_;
    pid_t pid_;
};

pic::tool_t::tool_t(const char *dir,const char *name)
{
    impl_ = new impl_t(dir,name);
}

pic::tool_t::tool_t(const std::string &dir,const char *name)
{
    impl_ = new impl_t(dir,name);
}

pic::tool_t::~tool_t()
{
    delete impl_;
}

bool pic::tool_t::isrunning()
{
    return impl_->is_running();
}

void pic::tool_t::start()
{
    impl_->start();
}

void pic::tool_t::quit()
{
    impl_->quit();
}
