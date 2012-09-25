
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
#include <lib_juce/ejuce_laf.h>
#include <picross/pic_resources.h>

#include "JucerScanCtlComponent.h"
#include "JucerScanPrgComponent.h"
#include "JucerScanCompleteComponent.h"
#include <picross/pic_tool.h>
#include <picross/pic_power.h>

#include <memory>

class EigenScanner;

#define STATE_STARTING 0 // waiting for scanner process
#define STATE_SCANNING 1 // scanning plugin

#ifdef PI_MACOSX
#define SCAN_HELPER_NAME "EigenScanner0"
#endif
#ifdef PI_WINDOWS
#define SCAN_HELPER_NAME "scanner0"
#endif
#ifdef PI_LINUX
#define SCAN_HELPER_NAME "scanner0"
#endif

class EigenScanner;
static AudioPluginFormatManager *get_manager(EigenScanner *app);

namespace
{
    File getPluginsDir()
    {
        return ejuce::pathToFile(pic::global_library_dir()).getChildFile("Plugins");
    }

    class EigenScanPrgComponent: public JucerScanPrgComponent
    {
    };


    class EigenScanPrgWindow: public DocumentWindow, public InterprocessConnection, public Timer
    {
        public:
            EigenScanPrgWindow(EigenScanner *app, const StringArray &skip,bool fullscan,const FileSearchPath &path): DocumentWindow("Plugin Scanner",Colours::black,DocumentWindow::allButtons,true), app_(app), tool_(pic::private_tools_dir(),SCAN_HELPER_NAME), skip_(skip), fullscan_(fullscan),path_(path)
            {
                component_ = new EigenScanPrgComponent();
                setContentOwned(component_,true);
                setUsingNativeTitleBar(true);
                setSize(400,90);
                setResizeLimits(1,1,400,90);
                setResizable(false,false);
                centreWithSize (getWidth(), getHeight());
                setVisible(true);
                toFront(true);

                createPipe("EigenScanner", -1);
                tool_.start();

                num_format_ = get_manager(app_)->getNumFormats();
                cur_format_ = -1;
                cur_plugin_ = -1;

                juce::File f(getPluginsDir().getChildFile("plugins_cache"));

                if(f.existsAsFile() && f.getSize()>0)
                {
                    XmlDocument xml(f);
                    current_plugin_list_.recreateFromXml(*xml.getDocumentElement());
                }
                else
                {
                    printf("no current plugin cache\n");
                }

                state_ = STATE_STARTING;
                timeout_ = 10;
                startTimer(1000);
            }

            void connectionMade() { }
            void connectionLost() { }

            void timerCallback()
            {
                //printf("timer %d\n",tool_.isrunning());

                if(!tool_.isrunning())
                {
                    event_crashed();
                    return;
                }

                if(timeout_ > 0)
                {
                    if(--timeout_ == 0)
                    {
                        event_timeout();
                    }
                }
            }

            void scan_next()
            {
                if(cur_plugin_ >= plugins_.size())
                {
                    cur_plugin_ = -1;
                }

                if(cur_plugin_ < 0)
                {
                    cur_format_++;

                    if(cur_format_ >= num_format_)
                    {
                        timeout_ = 0;
                        complete(true);
                        return;
                    }

                    AudioPluginFormat *format = get_manager(app_)->getFormat(cur_format_);
                    FileSearchPath path = format->getDefaultLocationsToSearch();

                    if(format->getName()=="VST")
                    {
                        path = path_;
                    }

                    plugins_ = format->searchPathsForPlugins(path,true);
                    cur_plugin_ = 0;
                    scan_next();
                    return;
                }


                bool scan = false;

                AudioPluginFormat *format = get_manager(app_)->getFormat(cur_format_);

                if(fullscan_)
                {
                    scan = true;
                }
                else
                {
                    PluginDescription *d = current_plugin_list_.getTypeForFile(plugins_[cur_plugin_]);

                    if(d)
                    {
                        plugin_list_.addType(*d);
                        printf("skip %s (good)\n",std::string(plugins_[cur_plugin_].getCharPointer()).c_str());

                        String name = format->getNameOfPluginFromIdentifier(plugins_[cur_plugin_]);
                        String goodmsg = plugins_[cur_plugin_];
                        goodmsg += " ## ";
                        goodmsg += name;
                        good_plugins_.add(goodmsg);
                    }
                    else
                    {
                        if(skip_.contains(plugins_[cur_plugin_]))
                        {
                            plugin_failed();
                            scan = false;
                            printf("skip %s (bad)\n",std::string(plugins_[cur_plugin_].getCharPointer()).c_str());
                        }
                        else
                        {
                            scan = true;
                            printf("rescan %s (bad)\n",std::string(plugins_[cur_plugin_].getCharPointer()).c_str());
                        }
                    }
                }

                String format_name = format->getName();
                String plugin_name = format->getNameOfPluginFromIdentifier(plugins_[cur_plugin_]);
                String msg = scan?"Scanning ":"Skipping "; msg += format_name;
                component_->getPluginLabel()->setText(plugin_name,false);
                component_->getFormatLabel()->setText(msg,false);
                float progress = ((float)cur_plugin_)/((float)plugins_.size());
                component_->getProgressBar()->setValue(progress,true);
                component_->getPeer()->performAnyPendingRepaintsNow();

                if(scan)
                {
                    msg = "F"; msg+=format->getName(); send(msg.toUTF8()); msg = "S"; msg+=plugins_[cur_plugin_]; send(msg.toUTF8());
                    state_ = STATE_SCANNING;
                    timeout_ = 10;
                }
                else
                {
                    ++cur_plugin_;
                    scan_next();
                }
            }

            void complete(bool ok);

            void send(const char *msg)
            {
                //printf("sending %s\n",msg);
                sendMessage(MemoryBlock(msg,1+strlen(msg)));
            }

            void plugin_failed()
            {
                AudioPluginFormat *format = get_manager(app_)->getFormat(cur_format_);
                String name = format->getNameOfPluginFromIdentifier(plugins_[cur_plugin_]);
                String msg = plugins_[cur_plugin_];
                msg += " ## ";
                msg += name;
                bad_plugins_.add(msg);
            }

            void plugin_loaded(const String &msg)
            {
                XmlDocument d(msg);
                PluginDescription info;
                info.loadFromXml(*d.getDocumentElement());
                plugin_list_.addType(info);

                AudioPluginFormat *format = get_manager(app_)->getFormat(cur_format_);
                String name = format->getNameOfPluginFromIdentifier(plugins_[cur_plugin_]);
                String goodmsg = plugins_[cur_plugin_];
                goodmsg += " ## ";
                goodmsg += name;
                good_plugins_.add(goodmsg);
            }

            void event_started()
            {
                if(state_ == STATE_STARTING)
                {
                    scan_next();
                    return;
                }
            }

            void event_failed()
            {
                if(state_ == STATE_SCANNING)
                {
                    plugin_failed();
                    cur_plugin_++;
                    scan_next();
                    return;
                }
            }

            void event_succeeded(const String &msg)
            {
                if(state_ == STATE_SCANNING)
                {
                    plugin_loaded(msg);
                    cur_plugin_++;

                    scan_next();
                    return;
                }
            }

            void event_timeout()
            {
                if(state_ == STATE_SCANNING)
                {
                    plugin_failed();
                    cur_plugin_++;
                    tool_.quit();
                    tool_.start();
                    state_ = STATE_STARTING;
                    timeout_ = 5;
                    return;
                }

                if(state_ == STATE_STARTING)
                {
                    tool_.quit();
                    complete(false);
                    return;
                }
            }

            void event_crashed()
            {
                if(state_ == STATE_STARTING)
                {
                    complete(false);
                    return;
                }

                if(state_ == STATE_SCANNING)
                {
                    plugin_failed();
                    cur_plugin_++;
                    tool_.quit();
                    tool_.start();
                    state_ = STATE_STARTING;
                    timeout_ = 5;
                }
            }

            void messageReceived (const MemoryBlock& message)
            {
                const char *msgtext = (const char *)message.getData();
                //printf("pipe message %s\n",msgtext);

                switch(msgtext[0])
                {
                    case 'B':
                        event_started();
                        return;

                    case '0':
                        event_failed();
                        return;

                    case '1':
                        event_succeeded(String::fromUTF8(&msgtext[1]));
                        return;
                }
            }

            void save()
            {
                juce::File f(getPluginsDir().getChildFile("plugins_cache"));

                std::auto_ptr<juce::XmlElement> el(plugin_list_.createXml());
                std::cout << "writing " << f.getFullPathName() << std::endl;
                if(!el->writeToFile(f,juce::String::empty)) std::cout << "oops, failed!" << std::cout;
                std::cout << "done." << std::endl;

                juce::File b(getPluginsDir().getChildFile("bad_plugins"));
                std::cout << "writing " << b.getFullPathName() << std::endl;
                if(!b.replaceWithText (bad_plugins_.joinIntoString ("\n"), true, true)) std::cout << "oops, failed!" << std::cout;
                std::cout << "done." << std::endl;
            }

            KnownPluginList &getPluginList() { return plugin_list_; }
            StringArray &getBadPlugins() { return bad_plugins_; }
            StringArray &getGoodPlugins() { return good_plugins_; }

        private:
            EigenScanPrgComponent *component_;
            EigenScanner *app_;
            pic::tool_t tool_;
            int num_format_;
            int cur_format_;
            int cur_plugin_;
            StringArray plugins_;
            int timeout_;
            int state_;
            KnownPluginList plugin_list_;
            KnownPluginList current_plugin_list_;
            StringArray bad_plugins_;
            StringArray current_bad_plugins_;
            StringArray good_plugins_;
            StringArray skip_;
            bool fullscan_;
            FileSearchPath path_;
            
            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EigenScanPrgWindow);
    };

    class PathModel: public ListBoxModel
    {
        public:
            PathModel(EigenScanner *app): font_(12.0)
            {
                File path_file(getPluginsDir().getChildFile("plugin_path"));

                if(path_file.exists())
                {
                    StringArray path;
                    path.addLines(path_file.loadFileAsString());

                    for(int i=0;i<path.size();i++)
                    {
                        path_.add(File(path[i]),0);
                    }
                }

                if(path_.getNumPaths())
                {
                    return;
                }

                for(int i=0;i<get_manager(app)->getNumFormats();i++)
                {
                    AudioPluginFormat *format = get_manager(app)->getFormat(i);

                    if(format->getName()=="VST")
                    {
                        path_ = format->getDefaultLocationsToSearch();
                        File vstpath = File(String::fromUTF8(pic::global_resource_dir().c_str()));
                        vstpath = vstpath.getChildFile("VST");
                        path_.add(vstpath);
                        break;
                    }
                }
            }

            void save()
            {
                File path_file(getPluginsDir().getChildFile("plugin_path"));
                StringArray path;

                for(int i=0;i<path_.getNumPaths();i++)
                {
                    path.add(path_[i].getFullPathName());
                }

                if(!path_file.replaceWithText (path.joinIntoString ("\n"), true, true)) printf("oops, failed!\n");
            }

            FileSearchPath path() { return path_; }

            void add_row(const File &dir)
            {
                path_.add(dir);
            }

            void del_row(int row)
            {
                path_.remove(row);
            }

            int getNumRows() { return path_.getNumPaths(); }

            void paintListBoxItem (int rowNumber, Graphics& g, int w, int h, bool rowIsSelected)
            {
                String name;

                if(rowNumber < path_.getNumPaths())
                {
                    name = path_[rowNumber].getFullPathName();
                }

                if (rowIsSelected) g.fillAll (Colours::black.withAlpha (0.2f));
                g.setColour(Colours::black);
                //g.setFont(font_);
                g.drawText(name,2,0,w-4,h,Justification::centredLeft,true);

            }

        private:
            FileSearchPath path_;
            Font font_;
    };

    class EigenScanCtlComponent: public JucerScanCtlComponent, public TableListBoxModel
    {
        public:
            EigenScanCtlComponent(EigenScanner *app): app_(app), font_(12.f), fullscan_(false), paths_(app)
            {
                table()->setModel(this);
                table()->getHeader().addColumn(String::empty,1,495,495,495,TableHeaderComponent::visible);
                table()->getHeader().addColumn(String::empty,2,17,17,17,TableHeaderComponent::visible);
                table()->setHeaderHeight(0);

                paths()->setModel(&paths_);

                File bad_file(getPluginsDir().getChildFile("bad_plugins"));

                if(bad_file.exists())
                {
                    bad_plugins_.addLines(bad_file.loadFileAsString());
                    rescan_bad_.resize(bad_plugins_.size());
                }
                else
                {
                    printf("no bad plugins file\n");
                    bad_file.create();
                }

                juce::File plugin_file(getPluginsDir().getChildFile("plugins_cache"));
                if(!plugin_file.exists())
                {
                    force_full_scan();
                }

                for(int i=0; i<bad_plugins_.size(); ++i)
                {
                    if(!bad_plugins_[i].contains(" ## "))
                    {
                        force_full_scan();
                        break;
                    }
                }
            }

            void force_full_scan()
            {
                full_scan_button()->setToggleState(true,true);
                full_scan_button()->setEnabled(false);
            }

            void buttonClicked (Button* buttonThatWasClicked);

            int getNumRows()
            {
                return bad_plugins_.size();
            }

            void paintCell(Graphics &g, int row, int col, int w, int h, bool sel)
            {
                if(col==1)
                {
                    g.setColour(Colours::black);
                    //g.setFont(font_);
                    g.drawText(bad_plugins_[row].fromFirstOccurrenceOf(" ## ",false,false),2,0,w-4,h,Justification::centredLeft,true);
                }
            }

            void paintRowBackground(Graphics& g, int row, int w, int h, bool sel) { if(sel) g.fillAll(Colours::lightgrey); }

            Component *refreshComponentForCell(int row, int col, bool sel, Component *existing)
            {
                if(col!=2)
                    return existing;

                ToggleButton *b = (ToggleButton *)existing;
                if(!b)
                {
                    b = new ToggleButton(String::empty);
                    b->addListener(this);
                }

                b->setToggleState(rescan_bad_[row],false);
                b->getProperties().set("__row",row);
                return b;
            }

            juce_UseDebuggingNewOperator

        private:
            EigenScanner *app_;
            Font font_;
            bool fullscan_;
            StringArray bad_plugins_;
            std::vector<bool> rescan_bad_;
            PathModel paths_;
    };

    class EigenScanCtlTable: public TableListBoxModel
    {
        public:
            EigenScanCtlTable(EigenScanner *app): app_(app)
            {
            }

        private:
            EigenScanner *app_;
    };

    class EigenScanCtlWindow: public DocumentWindow
    {
        public:
            EigenScanCtlWindow(EigenScanner *app): DocumentWindow("Plugin Scanner",Colours::black,DocumentWindow::allButtons,true), app_(app)
            {
                component_ = new EigenScanCtlComponent(app);
                setContentOwned(component_,true);
                centreWithSize (getWidth(), getHeight());
                setSize(component_->getWidth(),component_->getHeight());
                setResizable(false,false);
                setUsingNativeTitleBar(true);
                setResizeLimits(600,500,2000,2000);
                setVisible (true);
                toFront(true);
            }

            EigenScanner *app() { return app_; }

        private:
            EigenScanCtlComponent *component_;
            EigenScanner *app_;
    };

    class PluginListBox: public ListBoxModel
    {
        public:
            PluginListBox(const StringArray &list): list_(list), font_(12.0)
            {
            }

            int getNumRows() { return list_.size(); }

            void paintListBoxItem (int rowNumber, Graphics& g, int w, int h, bool rowIsSelected)
            {
                String name;

                if(rowNumber < list_.size())
                {
                    name = list_[rowNumber].fromFirstOccurrenceOf(" ## ",false,false);
                }

                g.setColour(Colours::black);
                //g.setFont(font_);
                g.drawText(name,2,0,w-4,h,Justification::centredLeft,true);

            }

            const StringArray list_;
            Font font_;
    };

    class EigenScanCompleteComponent: public JucerScanCompleteComponent
    {
        public:
            EigenScanCompleteComponent(bool ok, const StringArray &good,const StringArray &bad): good_model_(good), bad_model_(bad)
            {
                if(!ok)
                {
                    text()->setText("Scan failed.",false);
                    return;
                }

                if(!bad_model_.getNumRows())
                {
                    text()->setText("All plugins passed.",false);
                }
                else
                {
                    text()->setText("Scan complete, but some plugins failed:",false);
                }

                good_list_box()->setModel(&good_model_);
                failed_list_box()->setModel(&bad_model_);
            }

            void buttonClicked(Button *) { JUCEApplication::quit(); }

            PluginListBox good_model_, bad_model_;
    };

    class EigenScanCompleteWindow: public DocumentWindow
    {
        public:
            EigenScanCompleteWindow(bool ok, const StringArray &bad_plugins, const StringArray &good_plugins): DocumentWindow("Scan results",Colours::black,DocumentWindow::allButtons,true)
            {
                component_ = new EigenScanCompleteComponent(ok,good_plugins,bad_plugins);
                setContentOwned(component_,true);
                centreWithSize (getWidth(), getHeight());
                setSize(component_->getWidth(),component_->getHeight());
                setResizable(false,false);
                setUsingNativeTitleBar(true);
                setVisible (true);
                toFront(true);
            }


        private:
            EigenScanCompleteComponent *component_;
    };
};

class EigenScanner : public juce::JUCEApplication
{
    public:
        EigenScanner(): main_window_(0), scan_window_(0), complete_window_(0)
        {
        }

        ~EigenScanner()
        {
            shutdown();
        }

        void initialise (const String& commandLine)
        {
            pic::to_front();
            LookAndFeel::setDefaultLookAndFeel(new ejuce::EJuceLookandFeel);

            manager_.addDefaultFormats();

            main_window_ = new EigenScanCtlWindow(this);
            main_window_->toFront(true);
        }

        void shutdown()
        {
            if(main_window_) delete main_window_;
            main_window_ = 0;
            if(scan_window_) delete scan_window_;
            scan_window_ = 0;
            if(complete_window_) delete complete_window_;
            complete_window_ = 0;
        }

        const String getApplicationName()
        {
            return "EigenScanner";
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

        AudioPluginFormatManager *manager() { return &manager_; }

        void start(const StringArray &scan,bool fullscan,const FileSearchPath &path)
        {
            if(main_window_)
            {
                delete main_window_;
                main_window_ = 0;
            }

            scan_window_ = new EigenScanPrgWindow(this,scan,fullscan,path);
        }

        void complete(bool ok)
        {
            if(scan_window_)
            {
                if(ok)
                {
                    scan_window_->save();
                }

                StringArray bp = scan_window_->getBadPlugins();
                StringArray gp = scan_window_->getGoodPlugins();
                delete scan_window_;
                scan_window_ = 0;
                complete_window_ = new EigenScanCompleteWindow(ok,bp,gp);
            }
        }

    private:
        EigenScanCtlWindow *main_window_;
        EigenScanPrgWindow *scan_window_;
        EigenScanCompleteWindow *complete_window_;
        AudioPluginFormatManager manager_;
};

AudioPluginFormatManager *get_manager(EigenScanner *app)
{
    return app->manager();
}

void EigenScanCtlComponent::buttonClicked (Button* buttonThatWasClicked)
{
    if(buttonThatWasClicked==cancel_button())
    {
        JUCEApplication::quit();
        return;
    }

    if(buttonThatWasClicked==scan_button())
    {
        StringArray pluginsToSkip;

        if(!fullscan_)
        {
            for(int i=0; i<bad_plugins_.size(); ++i)
            {
                if(!rescan_bad_[i])
                {
                    String id = bad_plugins_[i].upToFirstOccurrenceOf(" ## ",false,false);
                    pluginsToSkip.add(id);
                }
            }
        }

        paths_.save();
        ((EigenScanCtlWindow *)getParentComponent())->app()->start(pluginsToSkip,fullscan_,paths_.path());
        return;
    }

    if(buttonThatWasClicked==full_scan_button())
    {
        fullscan_ = buttonThatWasClicked->getToggleState();
        table()->setVisible(!fullscan_);
        plugin_label()->setVisible(!fullscan_);
        return;
    }

    if(buttonThatWasClicked->getProperties().contains("__row"))
    {
        int row = buttonThatWasClicked->getProperties()["__row"];
        rescan_bad_[row] = buttonThatWasClicked->getToggleState();
        return;
    }

    if(buttonThatWasClicked==del_button())
    {
        int row = paths()->getLastRowSelected();

        if(row>=0)
        {
            paths_.del_row(row);
            paths()->updateContent();
        }

        return;
    }

    if(buttonThatWasClicked==add_button())
    {
        FileChooser f("VST Directory");
        bool t = f.browseForDirectory();

        if(t)
        {
            paths_.add_row(f.getResult());
            paths()->updateContent();
            paths()->updateContent();
        }
        return;
    }
}

void EigenScanPrgWindow::complete(bool ok)
{
    app_->complete(ok);
}

START_JUCE_APPLICATION (EigenScanner)
