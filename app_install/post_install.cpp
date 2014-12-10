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


#include <piw/piw_tsd.h>
#include <picross/pic_power.h>
#include <picross/pic_time.h>
#include <picross/pic_resources.h>

#include <lib_juce/ejuce.h>
#include <lib_juce/ejuce_laf.h>
#include <lib_juce/epython.h>

#include "post_install.h"

#include "jucerInstallComponent.h"

class PostInstallApp;
class PostInstallWindow;

class PostInstallComponent: public jucerInstallComponent
{
    public:
        PostInstallComponent(PostInstallWindow *main, const String &new_version): main_(main), new_version_(new_version)
        {
            TextEditor *t2 = get_para1();
            Button *b2 = get_button1();

            if(!new_version_.isEmpty())
            {
                t2 = get_para2();
                b2 = get_button2();

                String tmpl = "There is a new EigenD version, VERSION, available which can be downloaded from Eigenlabs.com.  We advise you to upgrade to this as soon as is convenient.";

                tmpl = tmpl.replace("VERSION",new_version);
                get_button1()->setButtonText("Download Now");
                get_para1()->setText(tmpl);
            }
            else
            {
                get_button2()->setVisible(false);
                get_para2()->setVisible(false);
            }

            b2->setButtonText("Register Now");
            t2->setText("There is a great online community at Eigenlabs.com. Register your instrument today to join discussions, get access to support and download additional soundfonts.");
        }

        void buttonClicked (Button* b)
        {
            if(!new_version_.isEmpty())
            {
                if(b==get_button1())
                {
                    URL("http://www.eigenlabs.com/downloads/releases/state/stable/").launchInDefaultBrowser();
                    return;
                }

                if(b==get_button2())
                {
                    URL("http://www.eigenlabs.com/login/").launchInDefaultBrowser();
                    return;
                }
            }
            else
            {
                if(b==get_button1())
                {
                    URL("https://www.eigenlabs.com/login/").launchInDefaultBrowser();
                    return;
                }
            }

            JUCEApplication::quit();
        }

    private:
        PostInstallWindow *main_;
        String new_version_;
};

class PostInstallWindow: public DocumentWindow, public post_install::p2c_t
{
    public:
        PostInstallWindow(pia::scaffold_gui_t *scaffold, post_install::c2p_t *backend);
        ~PostInstallWindow();
        void closeButtonPressed();
        post_install::c2p_t *backend() { return backend_; }

        juce_UseDebuggingNewOperator

    private:
        PostInstallComponent *component_;
        pia::scaffold_gui_t *scaffold_;
        post_install::c2p_t *backend_;
};

class PostInstallApp : public ejuce::Application, virtual public pic::tracked_t
{
    public:
        PostInstallApp();
        ~PostInstallApp();
        void initialise (const String& commandLine);
        void shutdown();
        void log(const char *msg);
        const String getApplicationName();
        const String getApplicationVersion();
        bool moreThanOneInstanceAllowed();
        void anotherInstanceStarted (const String& commandLine);

    private:
        PostInstallWindow *main_window_;
        epython::PythonInterface *python_;
        pia::context_t context_;
};

START_JUCE_APPLICATION (PostInstallApp)


PostInstallWindow::PostInstallWindow(pia::scaffold_gui_t *scaffold, post_install::c2p_t *backend):
    DocumentWindow ("Post Install", Colours::black, DocumentWindow::allButtons, true),
    scaffold_(scaffold), backend_(backend)
{
    backend->initialise(this,scaffold);
    String new_url = String::fromUTF8(backend->get_new_version().c_str());

    component_ = new PostInstallComponent(this,new_url);
    setContentOwned(component_, true);
    centreWithSize (getWidth(), getHeight());
    setUsingNativeTitleBar(true);
    setVisible (true);
    pic::to_front();
    toFront(true);

}

PostInstallWindow::~PostInstallWindow()
{
}

void PostInstallWindow::closeButtonPressed()
{
    JUCEApplication::quit();
}

PostInstallApp::PostInstallApp(): main_window_ (0), python_(0)
{
}

PostInstallApp::~PostInstallApp()
{
}

void PostInstallApp::initialise (const String& commandLine)
{
    pic_set_interrupt();
    pic_mlock_code();
    pic_init_time();

    printf("release root: %s\n",pic::release_root_dir().c_str());

    pic::f_string_t logger = pic::f_string_t::method(this,&PostInstallApp::log);

    ejuce::Application::eInitialise(commandLine,logger,false,true);
    LookAndFeel::setDefaultLookAndFeel(new ejuce::EJuceLookandFeel);
    python_ = new epython::PythonInterface();
    context_ = scaffold()->context("main",pic::status_t(),logger,"PostInstallApp");

    piw::tsd_setcontext(context_.entity());
    python_->py_startup();

    if(python_->init_python("app_install.post_install","main"))
    {
        post_install::c2p_t *backend = (post_install::c2p_t *)python_->mediator();
        backend->set_args(commandLine.toUTF8());
        main_window_ = new PostInstallWindow(scaffold(),backend);
    }
    else
    {
        juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon, "An unexpected error occurred ...", python_->last_error().c_str());
    }
}

void PostInstallApp::shutdown()
{
    if (main_window_ != 0) delete main_window_;

    ejuce::Application::shutdown();
}

const String PostInstallApp::getApplicationName()
{
    return "PostInstallApp";
}

const String PostInstallApp::getApplicationVersion()
{
    return pic::release().c_str();
}

bool PostInstallApp::moreThanOneInstanceAllowed()
{
    return false;
}

void PostInstallApp::log(const char *msg)
{
    printf("%s\n",msg);
    fflush(stdout);
}

void PostInstallApp::anotherInstanceStarted (const String& commandLine)
{
    pic::logmsg() << "new instance";
}
