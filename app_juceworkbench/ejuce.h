
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
#include <picross/pic_functor.h>
#include <picross/pic_config.h>
#include "juce.h"

namespace ejuce
{
    class Application: public juce::JUCEApplication
    {
        private:
            class impl_t;

        public:
            Application();
            ~Application();
            void eInitialise (const juce::String& commandLine,const pic::f_string_t &,bool ck, bool rt);
            void shutdown();
            virtual void handleGone();
            virtual void handleWinch(const std::string &);
            pia::scaffold_gui_t *scaffold();

        private:
            impl_t *messages_;
    };

    juce::File pathToFile(const std::string &path);
    juce::File pathToFile(const char* path);
};
