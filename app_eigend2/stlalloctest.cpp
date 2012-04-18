
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

#include <piagent/pia_scaffold.h>
#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>
#include <picross/pic_power.h>
#include <picross/pic_thread.h>
#include <picross/pic_time.h>
#include <picross/pic_tool.h>
#include <picross/pic_resources.h>
#include <picross/pic_flipflop.h>
#include <picross/pic_stl.h>
#include <picross/pic_fastalloc.h>

#include <piw/piw_data.h>

#include <lib_juce/ejuce.h>

#define THREADS 8

typedef pic::lckmap_t<piw::data_t,void *,piw::path_less>::lcktype test_map_t;
typedef pic::flipflop_t<test_map_t> test_map_flipflop_t;

class stl_thread_t: pic::thread_t
{
    public:
        stl_thread_t(unsigned index) : index_(index)
        {
            snapshot_.save();
        }

        ~stl_thread_t() {}

        void start() { run(); }

        void thread_main()
        {
            for(unsigned id = 0;;++id)
            {
                piw::data_t path = piw::pathtwo(1+id/255,1+id%255,0);
                //printf("%2u inserting path for id %u\n", index_, id);
                map_.alternate().insert(std::make_pair(path,this));
                map_.exchange();
            }
        }

        void thread_init()
        {
            snapshot_.install();
        }

        void thread_term() {}

    private:
        unsigned index_;
        piw::tsd_snapshot_t snapshot_;
        test_map_flipflop_t map_;
};

class raw_thread_t: pic::thread_t
{
    public:
        raw_thread_t(unsigned index) : index_(index)
        {
            snapshot_.save();
        }

        ~raw_thread_t() {}

        void start() { run(); }

        void thread_main()
        {
            pic::nballocator_t *a = pic::nballocator_t::tsd_getnballocator();
            for(;;)
            {
                pic::nballocator_t::deallocator_t dealloc;
                void *dealloc_arg;

                unsigned s = 1+random()%4096;
                void *m = a->allocator_xmalloc(PIC_ALLOC_NB,s,&dealloc,&dealloc_arg);

                dealloc(m,dealloc_arg);
            }
        }

        void thread_init()
        {
            snapshot_.install();
        }

        void thread_term() {}

    private:
        unsigned index_;
        piw::tsd_snapshot_t snapshot_;
};

class Test: public ejuce::Application, virtual public pic::tracked_t
{
    public:
        Test() {}
        ~Test() {}

        void initialise(const juce::String& commandLine)
        {
            pic_set_interrupt();
            pic_mlock_code();
            pic_init_time();

            pic::f_string_t test_logger = pic::f_string_t::method(this,&Test::log);

            ejuce::Application::initialise(commandLine,test_logger,false,true);

            context_ = scaffold()->context("main",pic::status_t(),test_logger,"eigend");

            piw::tsd_setcontext(context_.entity());

            for(unsigned i=0; i<THREADS; ++i)
            {
                raw_thread_t *t = new raw_thread_t(i);
                t->start();
            }
        }

        void shutdown()
        {
            ejuce::Application::shutdown();
        }

        void log(const char *msg)
        {
            printf("%s\n",msg);
            fflush(stdout);
        }

        const juce::String getApplicationName()
        {
            return T("test");
        }

        const juce::String getApplicationVersion()
        {
            return "1.0";
        }

        void handleWinch(const std::string &msg)
        {
            log(msg.c_str());
        }

    private:
        pia::context_t context_;
};

START_JUCE_APPLICATION (Test)
