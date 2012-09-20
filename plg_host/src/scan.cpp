
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

#include <lib_juce/ejuce.h>
#include <picross/pic_resources.h>
#include <picross/pic_power.h>
#include <memory>

using namespace juce;

namespace
{
    class Scanner: public juce::InterprocessConnection
    {
        public:
            Scanner(): format_(0)
            {
                manager_.addDefaultFormats();
                connectToPipe("EigenScanner", -1);
                send("B");
            }

            void connectionMade()
            {
            }

            void connectionLost()
            {
            }

            void messageReceived (const MemoryBlock& message)
            {
                const char *msgtext = (const char *)(message.getData());
                printf("pipe message %s\n",msgtext);

                if(msgtext[0]=='F')
                {
                    String msg = String::fromUTF8(&msgtext[1]);

                    format_ = 0;

                    for(int i=0;i<manager_.getNumFormats();i++)
                    {
                        AudioPluginFormat *f = manager_.getFormat(i);

                        if(msg==f->getName())
                        {
                            format_ = f;
                            return;
                        }
                    }
                }

                if(msgtext[0]=='S')
                {
                /*
                    if(!strcmp(msgtext,"SAudioUnit:Generators/augn,ttsp,appl"))
                    {
                        abort();
                    }

                    if(!strcmp(msgtext,"SAudioUnit:Panners/aupn,ambi,appl"))
                    {
                        for(;;) sleep(100);
                    }
                */

                    if(format_)
                    {
                        String msg = String::fromUTF8(&msgtext[1]);
                        OwnedArray <PluginDescription> found;
                        format_->findAllTypesForFile (found, msg);

                        if(found.size()>0)
                        {
                            std::auto_ptr<juce::XmlElement> el(found[0]->createXml());
                            msg = "1"; msg+= el->createDocument(juce::String::empty); send(msg.toUTF8());
                            return;
                        }
                    }

                    send("0");
                }
            }

            void send(const char *msg)
            {
                sendMessage(MemoryBlock(msg,1+strlen(msg)));
            }

        private:
            AudioPluginFormatManager manager_;
            AudioPluginFormat *format_;
    };
};

class EigenScanner : public juce::JUCEApplication
{
    public:
        EigenScanner()
        {
        }

        ~EigenScanner()
        {
        }

        void shutdown()
        {
        }

        void initialise(const juce::String &)
        {
            scanner_ = new Scanner();
            pic::to_front();
        }

        const String getApplicationName()
        {
            return "EigenScanner0";
        }

        const String getApplicationVersion()
        {
            return pic::release().c_str();
        }

        bool moreThanOneInstanceAllowed()
        {
            return false;
        }

        void anotherInstanceStarted (const String& commandLine)
        {
        }

    private:
        Scanner *scanner_;
};



START_JUCE_APPLICATION (EigenScanner)
