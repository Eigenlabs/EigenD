
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

#include "ejuce.h"
#include <picross/pic_time.h>

struct ejuce::Application::impl_t: public juce::MessageListener, virtual public pic::tracked_t
{
    impl_t(Application *app,const pic::f_string_t &,bool ck,bool rt);
    ~impl_t();
    void handleMessage(const juce::Message &message);
    void handleGone();
    void handleService();
    void handleWinch(const std::string &);

    ejuce::Application *application_;
    pia::scaffold_gui_t *scaffold_;
};


void ejuce::Application::impl_t::handleGone()
{
    application_->handleGone();
}

void ejuce::Application::impl_t::handleService()
{
    postMessage(new juce::Message());
}

void ejuce::Application::impl_t::handleMessage(const juce::Message &message)
{
    scaffold_->process_ctx();
}

void ejuce::Application::impl_t::handleWinch(const std::string &message)
{
    application_->handleWinch(message);
}

ejuce::Application::impl_t::impl_t(Application *app,const pic::f_string_t &l,bool ck,bool rt): application_(app)
{
    pic_init_time();
    scaffold_ = new pia::scaffold_gui_t(pic::notify_t::method(this,&impl_t::handleService), pic::notify_t::method(this,&impl_t::handleGone),l,pic::f_string_t::method(this,&impl_t::handleWinch),ck,rt);
}

ejuce::Application::impl_t::~impl_t()
{
    tracked_invalidate();
}

ejuce::Application::Application(): messages_(0)
{
}

ejuce::Application::~Application()
{
}

pia::scaffold_gui_t *ejuce::Application::scaffold()
{
    return messages_->scaffold_;
}

void ejuce::Application::eInitialise (const juce::String& commandLine,const pic::f_string_t &l,bool ck,bool rt)
{
    messages_ = new impl_t(this,l,ck,rt);
}

void ejuce::Application::shutdown()
{
    if(messages_)
        delete messages_;
}

void ejuce::Application::handleGone()
{
}

void ejuce::Application::handleWinch(const std::string &message)
{
}

juce::File ejuce::pathToFile(const std::string &path)
{
    return pathToFile(path.c_str());
}

juce::File ejuce::pathToFile(const char* path)
{
#ifndef PI_WINDOWS
    return juce::File(path);
#else
    int wchars_num = MultiByteToWideChar(CP_UTF8,0,path,-1,NULL,0);
    wchar_t* wstr = new wchar_t[wchars_num];
    MultiByteToWideChar(CP_UTF8,0,path,-1,wstr,wchars_num);
    juce::File result = juce::File(wstr);
    delete[] wstr;
    return result;
#endif
}
