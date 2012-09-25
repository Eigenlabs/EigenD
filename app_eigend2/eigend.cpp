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
#include <piw/piw_thing.h>
#include <picross/pic_power.h>
#include <picross/pic_thread.h>
#include <picross/pic_time.h>
#include <picross/pic_tool.h>
#include <picross/pic_resources.h>

#include "eigend.h"
#include "nettest.h"

#include <lib_juce/ejuce.h>
#include <lib_juce/ejuce_laf.h>
#include <lib_juce/epython.h>

#include "jucer/StatusComponent.h"
#include "jucer/LoadDialogComponent.h"
#include "jucer/LoadProgressComponent.h"
#include "jucer/SaveDialogComponent.h"
#include "jucer/EditDialogComponent.h"
#include "jucer/AboutComponent.h"
#include "jucer/BugComponent.h"
#include "jucer/AlertComponent1.h"
#include "jucer/ModalComponent.h"
#include "jucer/AlertComponent2.h"
#include "jucer/HelpComponent.h"
#include "jucer/InfoComponent.h"

#include <algorithm>

static const char *save_help_label = "Help With Setups";
static const char *save_help_text = ""
"About Setups\n"
"\n"
"EigenD Setups contain everything about your current playing setup, including recordings that you have made using the built in recorders, which scales and instruments you have selected or loaded (including AU/VST instruments) and the settings of those instruments. \n"
"\n"
"EigenD Setups are not stored using filenames but instead as a Belcanto name. This is so that you can recall and save setups directly from the instrument using Belcanto, from the Commander, keys on the instrument or preprogrammed Talker keys to do this for you. The standard MIDI IN mapping includes a Talker that recalls User Setups 1 to 10 for you, mapped to notes from C0 to A0.\n"
"\n"
"You will find that when you load setups, the amount of time this takes depends on how different they are. Setups that only differ in a few ways, for example some recordings or scale changes, will only take a few seconds to load. Setups that have different AU or VST instruments loaded or different Soundfonts in the samplers will take longer. Setups that have different key layouts or arrangements of instruments will normally take the longest. Loading individual Setups for the first time after a software upgrade may take a little longer as the Setup may need to be updated to run with the new software.\n"
"\n"
"\n"
"Using Basic Setups\n"
"\n"
"The simplest way to use Setups is to store them as a User Setup number. You can do this by using the top option on the Save panel. Pressing 'New' finds the next available unused number and assigns it for you.\n"
"\n"
"You can add text tags to your saved Setup to help you find it again later. You can, for example, use the name of the song you are playing as a tag. You can add a long description as well.\n"
"\n"
"\n"
"Using Advanced Setups\n"
"\n"
"If you want to use a full Belcanto name to save a setup you can use the second and third options in the Save panel. \n"
"\n"
"Words typed in as text must exist in the Belcanto Dictionary (which can be seen in the EigenD Commander). If you want to save a Setup by using Belcanto notes, these must be from 1 to 8. If you are saving a Setup using notes you are not confined to Belcanto words in the dictionary. You can use any sequence of notes in the major scale (typed in a numbers 1-8) as words. If they appear in the Dictionary, you will see the text translation appear in the text input box. Words that have no translation into text will appear with a '!' in front of them to let you know that they are not a number but a new Belcanto word. \n"
"\n"
"When you go to load a Setup you will see that all your saved Setups are arranged in a hierarchy which uses each part of the Belcanto name as a level. This, along with the tags, helps you to organise and remember large collections of Setups.\n";

#ifdef PI_MACOSX
#define TOOL_BROWSER "EigenBrowser"
#define TOOL_COMMANDER "EigenCommander"
#define TOOL_SCANNER "EigenScanner"
#define TOOL_STAGE "Stage"
#define TOOL_WORKBENCH "Workbench"
#endif

#ifdef PI_WINDOWS
#define TOOL_BROWSER "EigenBrowser"
#define TOOL_COMMANDER "EigenCommander"
#define TOOL_SCANNER "Scanner"
#define TOOL_STAGE "Stage"
#define TOOL_WORKBENCH "Workbench"
#endif

#ifdef PI_LINUX
#define TOOL_BROWSER "eigenbrowser"
#define TOOL_COMMANDER "eigencommander"
#define TOOL_SCANNER "scanner"
#define TOOL_STAGE "Stage"
#define TOOL_WORKBENCH "Workbench"
#endif

class EigenD;
class EigenLoadComponent;

class EigenBugComponent;
class EigenMainWindow;

namespace
{
    enum EigenDMessageTypes 
    {
        messageEscape,
        messageConfirm
    };
    
    struct EigenDMessage: juce::Message
    {
        EigenDMessage (const int type): type_ (type) {}
        ~EigenDMessage() {}
        
        const int type_;
    };
}

enum EigenCommands
{
    commandAbout = 1000,
    commandStartStatus,
    commandStartSave,
    commandStartSaveAs,
    commandStartLoad,
    commandStartBug,
    commandStartBrowser,
    commandStartCommander,
    commandStartScanner,
    commandStartStage,
    commandStartWorkbench,
    commandLibraryDirectory,
    commandMainWindow,
    commandQuit,
    commandDownload,
    commandResetWarnings,
    commandWindow // must be last
};

class EigenAlertDelegate
{
    public:
        virtual ~EigenAlertDelegate() {}
        virtual void alert_ok() {}
        virtual void alert_cancel() {}
};

class EigenDialogContent
{
    public:
        virtual bool returnKeyPressed() { return false; }
};

class EigenAlertComponent2: public AlertComponent2, EigenDialogContent
{
    public:
        EigenAlertComponent2(EigenMainWindow *main,const String &klass,const String &label, const String &text, EigenAlertDelegate *listener);
        ~EigenAlertComponent2();
        void set_listener(EigenAlertDelegate *l);
        void buttonClicked (Button* b);
        bool returnKeyPressed() { buttonClicked(get_ok_button()); return true; }

    private:
        EigenMainWindow *main_;
        EigenAlertDelegate *listener_;
};

class EigenAlertComponent1: public AlertComponent1, public EigenDialogContent
{
    public:
        EigenAlertComponent1(EigenMainWindow *main,const String &klass,const String &label, const String &text);
        ~EigenAlertComponent1();
        void buttonClicked (Button* buttonThatWasClicked);
        bool returnKeyPressed() {  buttonClicked(get_ok_button()); return true; }

    private:
        EigenMainWindow *main_;
};

class EigenInfoComponent: public InfoComponent, public EigenDialogContent
{
    public:
        EigenInfoComponent(EigenMainWindow *main,const String &klass,const String &label, const String &text);
        ~EigenInfoComponent();
        void buttonClicked (Button* buttonThatWasClicked);
        bool returnKeyPressed() {  buttonClicked(get_ok_button()); return true; }

    private:
        EigenMainWindow *main_;
};

class EigenLoadProgressComponent: public LoadProgressComponent, public EigenDialogContent
{
    public:
        EigenLoadProgressComponent() {}
        ~EigenLoadProgressComponent() {}

    private:
        EigenMainWindow *main_;
};

class EigenHelpComponent: public HelpComponent, public EigenDialogContent
{
    public:
        EigenHelpComponent(EigenMainWindow *main,const String &klass,const String &label, const String &text);
        ~EigenHelpComponent();
        void buttonClicked (Button* buttonThatWasClicked);
        bool returnKeyPressed() {  buttonClicked(get_ok_button()); return true; }

    private:
        EigenMainWindow *main_;
};

class EigenLogger
{
    public:
        EigenLogger(const char *prefix, const pic::f_string_t &logger);
        EigenLogger(const EigenLogger &l);
        EigenLogger &operator=(const EigenLogger &l);
        bool operator==(const EigenLogger &l) const;
        static pic::f_string_t create(const char *prefix, const pic::f_string_t &logger);
        void operator()(const char *msg) const;

    private:
        juce::String prefix_;
        pic::f_string_t logger_;
};

class EigenDialog: public DocumentWindow, public KeyListener, public MessageListener
{
    public:
        EigenDialog(EigenMainWindow *main, Component *content, int, int, int, int, int, int, Component *position = 0, int buttons = DocumentWindow::closeButton|DocumentWindow::minimiseButton);
        void closeButtonPressed();
        ~EigenDialog();
        bool keyPressed (const KeyPress& key, Component* originatingComponent);
        bool keyStateChanged (bool isKeyDown, Component* originatingComponent);
        void handleMessage(const Message &m);

    private:
        EigenMainWindow *main_;
        Component *component_;
};

class EigenMainWindow: public DocumentWindow, public MenuBarModel, public piw::thing_t, public eigend::p2c_t, public ApplicationCommandTarget
{
    public:
        EigenMainWindow(ApplicationCommandManager *mgr, pia::scaffold_gui_t *scaffold, eigend::c2p_t *backend, const pic::f_string_t &logger);
        ~EigenMainWindow();
        void getAllCommands (Array <CommandID>& commands);
        void getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result);
        ApplicationCommandTarget *getNextCommandTarget();
        PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& /*menuName*/);
        bool perform (const InvocationInfo& info);
        void thing_timer_slow();
        void set_cpu(unsigned cpu);
        void load_started(const char *setup);
        void load_status(const char *msg,unsigned progress);
        void load_ended();
        pic::f_string_t make_logger(const char *prefix);
        void closeButtonPressed();
        void windowClosed(Component *window);
        ApplicationCommandManager *manager() { return manager_; }
        eigend::c2p_t *backend() { return backend_; }
        void init_menu();
        StringArray getMenuBarNames();
        void menuItemSelected  (int menuItemID, int topLevelMenuIndex);
        void setups_changed(const char *file);
        EigenDialog *alert1(const String &klass, const String &label, const String &text, bool left=false);
        EigenDialog *alert2(const String &klass, const String &label, const String &text, EigenAlertDelegate *listener);
        EigenDialog *info(const String &klass, const String &label, const String &text);
        void ignore_klass(const juce::String &klass);
        bool select_setup(const char *setup);
        void load_setup(const char *setup, bool user);
        void save();
        void save_dialog();
        void save_dialog(const std::string &current);
        void edit_dialog(const std::string &current);
        void alert_dialog(const char *klass, const char *label, const char *text);
        void info_dialog(const char *klass, const char *label, const char *text);
        void dialog_dead(EigenDialog *dialog);
        void show_save_help(bool);
        void set_latest_release(const char *);
        std::string get_cookie();
        std::string get_os();
        void enable_save_menu(bool active);
        bool do_quit();

    private:
        EigenLoadComponent *component_;
        ApplicationCommandManager *manager_;
        pia::scaffold_gui_t *scaffold_;
        eigend::c2p_t *backend_;
        EigenDialog *status_;
        EigenDialog *saving_;
        EigenDialog *editing_;
        EigenDialog *about_;
        pic::f_string_t logger_;
        EigenDialog *bug_;
        pic::tool_t browser_;
        pic::tool_t commander_;
        pic::tool_t scanner_;
        pic::tool_t workbench_;
        pic::tool_t stage_;
        std::string new_version_;
        std::string current_setup_;
        std::map<std::string,EigenDialog *> alert_dialogs_;
        std::map<std::string,EigenDialog *> info_dialogs_;
        EigenDialog *progress_;
        EigenDialog *help_;
        juce::PropertiesFile ignores_;
        bool save_menu_active_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EigenMainWindow);
};

class EigenD : public ejuce::Application, virtual public pic::tracked_t
{
    public:
        EigenD();
        ~EigenD();
        void initialise (const String& commandLine);
        void shutdown();
        const String getApplicationName();
        const String getApplicationVersion();
        bool moreThanOneInstanceAllowed();
        void anotherInstanceStarted (const String& commandLine);
        void log(const char *msg);
        void handleWinch(const std::string &msg);
        void systemRequestedQuit();

    private:
        EigenMainWindow * volatile main_window_;
        epython::PythonInterface *python_;
        pia::context_t context_;
        FILE *logfile_;
        pic::lockfile_t lockfile_;
};

class EigenSaveComponent: public SaveDialogComponent, public EigenAlertDelegate, public EigenDialogContent
{
    public:
        EigenSaveComponent(EigenMainWindow *mediator, const std::string &current);
        virtual ~EigenSaveComponent();
        void buttonClicked (Button* buttonThatWasClicked);
        void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
        void alert_ok();
        void alert_cancel();
        void cancel_confirm();
        unsigned getUserNumber(const juce::String &tag);

    private:
        EigenMainWindow *mediator_;
        EigenDialog *confirm_;
        piw::term_t term_;
        int max_;
};

class EigenEditComponent: public EditDialogComponent, public EigenDialogContent
{
    public:
        EigenEditComponent(EigenMainWindow *mediator, const std::string &current);
        virtual ~EigenEditComponent();
        void buttonClicked (Button* buttonThatWasClicked);

    private:
        EigenMainWindow *mediator_;
        piw::term_t term_;
        std::string orig_;
};

class EigenBugComponent: public BugComponent, public EigenDialogContent
{
    public:
        EigenBugComponent(EigenMainWindow *mediator);
        virtual ~EigenBugComponent();
        void buttonClicked (Button* buttonThatWasClicked);

    private:
        EigenMainWindow *mediator_;
};

class EigenLoadComponent;

class EigenTreeItem: public TreeViewItem
{
    public:
        EigenTreeItem (EigenLoadComponent *view, const piw::term_t &term);
        ~EigenTreeItem();
        void itemSelectionChanged(bool isNowSelected);
        void itemDoubleClicked(const MouseEvent &);
        int getItemWidth() const;
        bool mightContainSubItems();
        void paintItem (Graphics& g, int width, int height);
        void itemOpennessChanged (bool isNowOpen);
        void tree_changed(const piw::term_t &term);
        bool select_setup(const char *setup);
        piw::data_t name();
        juce::String getSlot();

    private:
        piw::term_t term_;
        EigenLoadComponent *view_;
};

class EigenLoadComponent: public LoadDialogComponent, public EigenAlertDelegate
{
    public:
        EigenLoadComponent(EigenMainWindow *mediator);
        void updateSaveButton(bool enabled);
        void updateSetupButtons(bool term, bool user);
        void selected(const piw::term_t &term, bool dbl);
        void buttonClicked(Button *b);
        void tree_changed();
        virtual ~EigenLoadComponent();
        EigenMainWindow *mediator() { return mediator_; }
        bool select_setup(const char *setup);
        void clear_selection();
        bool is_selected_user();
        void alert_ok();
        void alert_cancel();
        void cancel_confirm();

    private:
        EigenMainWindow *mediator_;
        EigenDialog *confirm_;
        EigenTreeItem *model_;
        piw::data_t slot_;
        piw::data_t selected_;
        bool user_;
};

void EigenTreeItem::itemSelectionChanged(bool isNowSelected)
{
    if(isNowSelected)
    {
        if(term_.arity()>2)
        {
            view_->selected(term_,false);
        }
        else
        {
            view_->clear_selection();
        }
    }
}

void EigenTreeItem::itemDoubleClicked(const MouseEvent &)
{
    if(term_.arity()>2)
    {
        view_->selected(term_,true);
    }
    else
    {
        view_->updateSetupButtons(false, false);
    }
}

piw::data_t EigenTreeItem::name()
{
    return term_.arg(0).value();
}

class EigenStatusComponent: public StatusComponent, public EigenDialogContent
{
    public:
        EigenStatusComponent(EigenMainWindow *mediator);
        virtual ~EigenStatusComponent();

    private:
        EigenMainWindow *mediator_;
};

class EigenAboutComponent: public AboutComponent, public EigenDialogContent
{
    public:
        EigenAboutComponent(EigenMainWindow *mediator);
        virtual ~EigenAboutComponent();

    private:
        EigenMainWindow *mediator_;
};

bool EigenMainWindow::perform (const InvocationInfo& info)
{
    if(info.commandID>=commandWindow)
    {
        unsigned w = info.commandID-commandWindow+1;
        scaffold_->set_window_state(w-1,!scaffold_->window_state(w-1));
        return true;
    }

    switch (info.commandID)
    {
        case commandAbout:
            if(!about_)
            {
                about_ = new EigenDialog(this,new EigenAboutComponent(this),300,400,0,0,0,0,this);
            }
            break;

        case commandStartStatus:
            if(!status_)
            {
                status_ = new EigenDialog(this,new EigenStatusComponent(this),250,24,250,50,2000,50);
            }
            else
            {
                delete status_;
                status_=0;
            }
            menuItemsChanged();
            break;

        case commandStartCommander:
            commander_.start();
            break;

        case commandStartBrowser:
            browser_.start();
            break;

        case commandStartWorkbench:
            workbench_.start();
            break;

        case commandStartScanner:
            scanner_.start();
            break;

        case commandStartStage:
            stage_.start();
            break;
            
        case commandStartBug:
            if(!bug_)
            {
                bug_ = new EigenDialog(this,new EigenBugComponent(this),700,700,700,700,2000,2000,this);
            }
            break;

        case commandStartSave:
            save();
            break;

        case commandStartSaveAs:
            save_dialog(current_setup_);
            break;

        case commandStartLoad:
            if(!isVisible())
            {
                setVisible(true);
                menuItemsChanged();
            }
            break;

        case commandLibraryDirectory:
            {
                ejuce::pathToURL(pic::global_library_dir()).launchInDefaultBrowser();
            }
            break;

        case commandMainWindow:
#ifdef JUCE_MAC
            setVisible(!isVisible());
            menuItemsChanged();
#else
            setMinimised(!isMinimised());
#endif
            break;

        case commandQuit:
            JUCEApplication::getInstance()->systemRequestedQuit();
            break;

        case commandDownload:
            {
                if(workbench_.isavailable())
                {
                    URL u("http://www.eigenlabs.com/downloads/latest/pro/");
                    u.launchInDefaultBrowser();
                }
                else
                {
                    URL u("http://www.eigenlabs.com/downloads/latest/std/");
                    u.launchInDefaultBrowser();
                }
            }
            break;

        case commandResetWarnings:
            ignores_.clear();
            break;
    }

    return true;
}

void EigenLoadComponent::updateSetupButtons(bool term, bool user)
{
    getDefaultToggle()->setEnabled(term);
    getLoadButton()->setEnabled(term);
    getEditButton()->setEnabled(term && user);
    getDeleteButton()->setEnabled(term && user);
}

void EigenLoadComponent::updateSaveButton(bool enabled)
{
    getSaveButton()->setEnabled(enabled);
}

void EigenLoadComponent::selected(const piw::term_t &term,bool dbl)
{
    slot_ = term.arg(3).value();
    selected_ = term.arg(4).value();
    user_ = term.arg(6).value().as_bool();

    std::string d = mediator_->backend()->get_description(selected_.as_string());
    getLabel()->setText(d.c_str(),false);

    updateSetupButtons(true,user_);

    std::string default_setup = mediator_->backend()->get_default_setup(false);
    getDefaultToggle()->setToggleState(!default_setup.compare(selected_.as_string()),false);

    if(dbl)
    {
        buttonClicked(getLoadButton());
    }
}

void EigenMainWindow::show_save_help(bool show)
{
    if(show)
    {
        if(!help_)
        {
            help_ = new EigenDialog(this,new EigenHelpComponent(this,save_help_label,save_help_label,save_help_text),500,600,500,600,2000,2000,this);
        }
    }
    else
    {
        if(help_)
        {
            delete help_;
            help_=0;
        }
    }
}

void EigenLoadComponent::buttonClicked(Button *b)
{
    if(b==getLoadButton())
    {
        if(selected_.is_string())
        {
            mediator_->load_setup(selected_.as_string(),user_);
        }
    }
    else if (b==getSaveButton())
    {
        mediator_->save();
    }
    else if (b==getSaveAsButton())
    {
        mediator_->save_dialog();
    }
    else if (b==getEditButton())
    {
        mediator_->edit_dialog(selected_.as_string());
    }
    else if (b==getDeleteButton())
    {
        juce::String msg = "This will delete a setup named\n\n";
        msg += slot_.as_string();
        confirm_ = mediator_->alert2("Delete Setup","Delete Setup",msg,this);
    }
    else if (b==getDefaultToggle())
    {
        if(b->getToggleState())
        {
            mediator_->backend()->set_default_setup(selected_.as_string());
        }
        else
        {
            mediator_->backend()->set_default_setup("");
        }
    }
}

EigenLoadComponent::~EigenLoadComponent()
{
    cancel_confirm();
}

void EigenLoadComponent::cancel_confirm()
{
    if(confirm_)
    {
        ((EigenAlertComponent2 *)(confirm_->getContentComponent()))->set_listener(0);
        delete confirm_;
        confirm_=0;
    }
}

EigenLoadComponent::EigenLoadComponent(EigenMainWindow *mediator): mediator_(mediator), confirm_(0), user_(false)
{
    setName("Load");
    model_ = new EigenTreeItem(this,mediator_->backend()->get_setups());
    model_->setOpen(true);
    getTreeView()->setMultiSelectEnabled(false);
    getTreeView()->setRootItem(model_);
    getLabel()->setText("",false);
}

void EigenLoadComponent::alert_ok()
{
    mediator_->backend()->delete_setup(slot_.as_string());
    confirm_ = 0;
}

void EigenLoadComponent::alert_cancel()
{
    confirm_ = 0;
}

bool EigenLoadComponent::is_selected_user()
{
    return user_;
}

bool EigenLoadComponent::select_setup(const char *setup)
{
    bool rv = model_->select_setup(setup);
    model_->setOpen(true);

    getSetupLabel()->setText("",0);

    if(rv)
    {
        EigenTreeItem *i = (EigenTreeItem *)(getTreeView()->getSelectedItem(0));

        if(i)
        {
            getSetupLabel()->setText(i->getSlot(),false);
        }
    }

    return rv;
}

void EigenLoadComponent::clear_selection()
{
    getDefaultToggle()->setToggleState(false,false);
    updateSetupButtons(false, false);
    slot_ = piw::data_t();
    selected_ = piw::data_t();
    user_ = false;
}

void EigenLoadComponent::tree_changed()
{
    typedef std::pair<TreeViewItem*, std::string> ItemStackEntry;

    // traverse the tree to collect the opened branches
    std::vector<ItemStackEntry> items;
    std::vector<std::string> opened;

    items.push_back(ItemStackEntry(model_,""));
    while(!items.empty())
    {
        ItemStackEntry current=items.back();
        items.pop_back();
        for(int i=0;i<current.first->getNumSubItems();i++)
        {
            TreeViewItem *item=current.first->getSubItem(i);
            std::string name=current.second+"\t"+std::string(((EigenTreeItem*)item)->name().as_string());
            if(item->isOpen())
            {
                opened.push_back(name);
            }
            items.push_back(ItemStackEntry(item,name));
        }
    }

    model_->tree_changed(mediator_->backend()->get_setups());

    // traverse the tree to restore the opened branches
    std::vector<std::string>::iterator it;
    items.clear();

    items.push_back(ItemStackEntry(model_,""));
    while(!items.empty())
    {
        ItemStackEntry current=items.back();
        items.pop_back();
        for(int i=0;i<current.first->getNumSubItems();i++)
        {
            TreeViewItem *item=current.first->getSubItem(i);
            std::string name=current.second+"\t"+std::string(((EigenTreeItem*)item)->name().as_string());
            if(opened.end()!=std::find(opened.begin(),opened.end(),name))
            {
                item->setOpen(true);
            }
            items.push_back(ItemStackEntry(item,name));
        }
    }
}

std::string EigenMainWindow::get_os()
{
    String os_name = juce::SystemStats::getOperatingSystemName();

    if(juce::SystemStats::isOperatingSystem64Bit())
    {
        os_name = os_name+" (64)";
    }

    std::string rv = std::string(os_name.getCharPointer());
    return rv;
}

std::string EigenMainWindow::get_cookie()
{
	juce::Array<MACAddress> mac_addresses;
	juce::MACAddress::findAllAddresses(mac_addresses);

    if(mac_addresses.size()>0)
    {
        std::string rv = std::string(mac_addresses[0].toString().getCharPointer());
        return rv;
    }

    return std::string();

}

void EigenMainWindow::enable_save_menu(bool active)
{
    save_menu_active_ = active;
    menuItemsChanged();
    component_->updateSaveButton(active);
}

void EigenMainWindow::set_latest_release(const char *release)
{
    JUCE_AUTORELEASEPOOL
    new_version_ = release;
    menuItemsChanged();
}

void EigenMainWindow::setups_changed(const char *file)
{
    JUCE_AUTORELEASEPOOL
    component_->tree_changed();
    if(file && file[0])
    {
        current_setup_ = file;
        select_setup(file);
        backend_->set_current_setup(file,component_->is_selected_user());
        enable_save_menu(component_->is_selected_user());
    }
    else
    {
        backend_->set_current_setup("",false);
        enable_save_menu(false);
    }
}

File getPluginsDir()
{
    return ejuce::pathToFile(pic::global_library_dir()).getChildFile("Plugins");
}

File getGlobalDir()
{
    return ejuce::pathToFile(pic::global_library_dir()).getChildFile("Global");
}

EigenMainWindow::EigenMainWindow(ApplicationCommandManager *mgr, pia::scaffold_gui_t *scaffold, eigend::c2p_t *backend, const pic::f_string_t &log):
    DocumentWindow ("eigenD", Colours::black, DocumentWindow::allButtons, true),
    manager_(mgr), scaffold_(scaffold), backend_(backend), status_(0), saving_(0), editing_(0), about_(0), logger_(log), bug_(0),
    browser_(pic::private_tools_dir(),TOOL_BROWSER), commander_(pic::private_tools_dir(),TOOL_COMMANDER), scanner_(pic::private_tools_dir(),TOOL_SCANNER), 
    workbench_(pic::public_tools_dir(),TOOL_WORKBENCH), stage_(pic::public_tools_dir(),TOOL_STAGE),
    progress_(0), help_(0), ignores_(getGlobalDir().getChildFile("ignores.xml"), PropertiesFile::Options()), save_menu_active_(false)
{
    backend->upgrade_setups();

    component_ = new EigenLoadComponent(this);
    setContentOwned(component_, true);
    centreWithSize (getWidth(), getHeight());
    setUsingNativeTitleBar(true);
    setResizable(true,true);
    setResizeLimits(600,585,2000,2000);
    setVisible(true);
    pic::to_front();
    toFront(true);
    component_->getSetupLabel()->setText("",false);

    piw::tsd_thing(this);
    timer_slow(500);

    pic::logmsg() << "latest version: " << new_version_;

    init_menu();

    manager_->registerAllCommandsForTarget(this);
    manager_->getKeyMappings()->resetToDefaultMappings();
    addKeyListener(manager_->getKeyMappings());
    mgr->setFirstCommandTarget(this);

    backend->initialise(this,scaffold,get_cookie(),get_os());

    std::string setup = backend->get_default_setup(true);

    if(setup.length())
    {
        if(select_setup(setup.c_str()))
        {
            pic::printmsg() << "default setup: " << setup;
            load_setup(setup.c_str(), component_->is_selected_user());
        }
    }

    juce::File plugin_file(getPluginsDir().getChildFile("plugins_cache"));
    if(!plugin_file.exists())
    {
        pic::logmsg() << "starting plugin scan..";
        scanner_.start();

        while(scanner_.isrunning())
        {
            juce::Thread::sleep(250);
        }

        pic::logmsg() << "plugin scan finished";
    }
}

EigenMainWindow::~EigenMainWindow()
{
    if (status_ != 0) delete status_;
    if (about_ != 0) delete about_;
    if (saving_ != 0) delete saving_;
    if (editing_ != 0) delete editing_;
    if (bug_ != 0) delete bug_;
}

bool EigenMainWindow::do_quit()
{
    if(!backend_->prepare_quit())
    {
        alert_dialog("Save In Progress","Save In Progress","A save operation is in progress.  Please wait for it to complete before quitting");
        return false;
    }

#ifdef JUCE_MAC
    MenuBarModel::setMacMainMenu(nullptr,nullptr);
#endif
    setMenuBar(nullptr);

    browser_.quit();
    commander_.quit();
    scanner_.quit();
    stage_.quit();
    workbench_.quit();
    setApplicationCommandManagerToWatch(0);
    backend_->quit();

    cancel_timer_slow();
    return true;
}

void EigenMainWindow::getAllCommands (Array <CommandID>& commands)
{
    const CommandID ids[] = { 
        commandAbout,
        commandStartStatus,
        commandStartSave,
        commandStartSaveAs,
        commandStartLoad,
        commandStartBug,
        commandStartBrowser,
        commandStartCommander,
        commandStartScanner,
        commandStartStage,
        commandStartWorkbench,
        commandMainWindow,
        commandLibraryDirectory,
        commandQuit,
        commandDownload,
        commandResetWarnings,
        commandWindow,
        commandWindow+1,
        commandWindow+2,
        commandWindow+3,
        commandWindow+4,
        commandWindow+5,
        commandWindow+6,
        commandWindow+7,
        commandWindow+8,
        commandWindow+9,
        commandWindow+10,
        commandWindow+11,
        commandWindow+12,
        commandWindow+13,
        commandWindow+14,
        commandWindow+15,
        commandWindow+16,
        commandWindow+17,
        commandWindow+18,
        commandWindow+19
    };

    commands.addArray (ids, numElementsInArray (ids));
}

void EigenMainWindow::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    const String generalCategory ("General");

    if(commandID>=commandWindow)
    {
        unsigned w = commandID-commandWindow+1;
        String j = String("Plugin Window ");
        j += String(w);
        result.setInfo (j,j, generalCategory, 0);
        if(w<10)
        {
            result.addDefaultKeypress (((char)(w+'0')), ModifierKeys::commandModifier);
        }
        result.setTicked(scaffold_->window_state(w-1));
        return;
    }

    switch (commandID)
    {
        case commandAbout:
            result.setInfo ("About eigenD", "Program Information", generalCategory, 0);
            break;

        case commandStartStatus:
            result.setInfo ("System Usage Meter", "System Usage Meter", generalCategory, 0);
            result.addDefaultKeypress ('x', ModifierKeys::commandModifier);
            result.setTicked(status_ && status_->isVisible());
            break;

        case commandStartCommander:
            result.setInfo ("EigenCommander", "EigenCommander", generalCategory, 0);
            result.addDefaultKeypress ('c', ModifierKeys::commandModifier);
            break;

        case commandStartBrowser:
            result.setInfo ("EigenBrowser", "EigenBrowser", generalCategory, 0);
            result.addDefaultKeypress ('b', ModifierKeys::commandModifier);
            break;

        case commandStartWorkbench:
            result.setInfo ("Workbench", "Workbench", generalCategory, 0);
            result.addDefaultKeypress ('k', ModifierKeys::commandModifier);
            result.setActive(workbench_.isavailable());
            break;

        case commandStartScanner:
            result.setInfo ("Plugin Scanner", "Plugin Scanner", generalCategory, 0);
            //result.addDefaultKeypress ('S', ModifierKeys::commandModifier);
            break;

        case commandStartStage:
            result.setInfo ("Stage", "Stage", generalCategory, 0);
            result.addDefaultKeypress ('g', ModifierKeys::commandModifier);
            break;
            
        case commandStartBug:
            result.setInfo ("Report Bug", "Report Bug", generalCategory, 0);
            result.addDefaultKeypress ('u', ModifierKeys::commandModifier);
            break;

        case commandStartLoad:
            result.setInfo ("Load Setup", "Load Setup", generalCategory, 0);
            result.addDefaultKeypress ('l', ModifierKeys::commandModifier);
            break;

        case commandStartSave:
            result.setInfo ("Save Setup", "Save Setup", generalCategory, 0);
            result.addDefaultKeypress ('s', ModifierKeys::commandModifier);
            result.setActive(save_menu_active_);
            break;

        case commandStartSaveAs:
            result.setInfo ("Save Setup As", "Save Setup As", generalCategory, 0);
            result.addDefaultKeypress ('s', ModifierKeys::commandModifier|ModifierKeys::shiftModifier);
            break;

        case commandLibraryDirectory:
            result.setInfo ("Open Library Directory", "Open Library Directory", generalCategory, 0);
            break;

        case commandMainWindow:
            result.setInfo ("Load Window", "Load Window", generalCategory, 0);
            result.addDefaultKeypress ('w', ModifierKeys::commandModifier);
            result.setTicked(isVisible());
            break;

        case commandQuit:
            result.setInfo ("Quit", "Quit eigenD", generalCategory, 0);
            result.addDefaultKeypress ('q', ModifierKeys::commandModifier);
            break;

        case commandDownload:
            {
                juce::String v = "Download "; v += juce::String(new_version_.c_str());
                result.setInfo (v, "Download New Version", generalCategory, 0);
                result.addDefaultKeypress ('d', ModifierKeys::commandModifier);
            }
            break;

        case commandResetWarnings:
            result.setInfo ("Reset Warnings", "Reset Warnings", generalCategory, 0);
            break;
    }
}

ApplicationCommandTarget *EigenMainWindow::getNextCommandTarget()
{
    return 0;
}

PopupMenu EigenMainWindow::getMenuForIndex (int topLevelMenuIndex, const String& /*menuName*/)
{
    PopupMenu menu;

    if (topLevelMenuIndex == 0)
    {
        menu.addCommandItem (manager(),commandStartLoad);
        menu.addCommandItem (manager(),commandStartSave);
        menu.addCommandItem (manager(),commandStartSaveAs);
        menu.addSeparator();
        menu.addCommandItem (manager(),commandStartBug);
        menu.addSeparator();
        menu.addCommandItem (manager(),commandAbout);
#ifndef PI_MACOSX
        menu.addSeparator();
        menu.addCommandItem (manager(),commandQuit);
#endif
    }

    if (topLevelMenuIndex == 1)
    {
        menu.addCommandItem (manager(),commandStartStatus);
        menu.addCommandItem (manager(),commandMainWindow);

        menu.addSeparator();
        unsigned n = scaffold_->window_count();

        for(unsigned i=0;i<n;i++)
        {
            std::string t = scaffold_->window_title(i);
            if(t.length()>0)
                menu.addCommandItem(manager(),commandWindow+i,t.c_str());
        }
    }

    if (topLevelMenuIndex == 2)
    {
        menu.addCommandItem (manager(),commandStartBrowser);
        menu.addCommandItem (manager(),commandStartCommander);
        menu.addCommandItem (manager(),commandStartStage);
        menu.addCommandItem (manager(),commandStartWorkbench);
        menu.addSeparator();
        menu.addCommandItem (manager(),commandStartScanner);
        menu.addSeparator();
        menu.addCommandItem (manager(),commandResetWarnings);
        menu.addCommandItem (manager(),commandLibraryDirectory);
    }

    if (topLevelMenuIndex == 3)
    {
        menu.addCommandItem (manager(),commandDownload);
    }

    return menu;
}


void EigenMainWindow::thing_timer_slow()
{
    set_cpu(scaffold_->cpu_usage());
}

void EigenMainWindow::set_cpu(unsigned cpu)
{
    if(status_)
    {
        ((StatusComponent *)(status_->getContentComponent()))->set_cpu(cpu);
        status_->getPeer()->performAnyPendingRepaintsNow();
    }
}

void EigenMainWindow::load_setup(const char *setup, bool user)
{
    backend_->load_setup(setup,user);
    enable_save_menu(user);
}

void EigenMainWindow::load_started(const char *setup)
{
    JUCE_AUTORELEASEPOOL

#ifdef DISABLE_FAST_THREAD_AT_LOAD
    scaffold_->fast_pause();
#endif

    if(progress_)
    {
        delete progress_;
        progress_ = 0;
    }

    EigenLoadProgressComponent *c = new EigenLoadProgressComponent();
    juce::String title = "Loading ";
    title += setup;
    c->setName(title);
    c->getMessageLabel()->setText("",false);
    c->getProgressSlider()->setValue(0,false,false);
    progress_ = new EigenDialog(this,c,400,90,0,0,0,0,this,DocumentWindow::minimiseButton);
    progress_->getPeer()->performAnyPendingRepaintsNow();
}

void EigenMainWindow::load_status(const char *msg,unsigned progress)
{
    JUCE_AUTORELEASEPOOL

    if(progress_)
    {
        LoadProgressComponent *c = (LoadProgressComponent *)(progress_->getContentComponent());
        c->getMessageLabel()->setText(msg,false);
        c->getProgressSlider()->setValue(progress,false,false);
        progress_->getPeer()->performAnyPendingRepaintsNow();
    }

}

void EigenMainWindow::load_ended()
{
    JUCE_AUTORELEASEPOOL

#ifdef DISABLE_FAST_THREAD_AT_LOAD
    scaffold_->fast_resume();
#endif

    if(progress_)
    {
        delete progress_;
        progress_ = 0;
    }
}

void EigenMainWindow::closeButtonPressed()
{
#ifdef JUCE_MAC
    setVisible(false);
#else
    setMinimised(!isMinimised());
#endif
}


void EigenMainWindow::init_menu()
{
#ifdef JUCE_MAC
    PopupMenu extra;
    extra.addCommandItem (manager(),commandAbout);
    MenuBarModel::setMacMainMenu(this,&extra);
#else
    setMenuBar(this);
#endif
}

StringArray EigenMainWindow::getMenuBarNames()
{
    if(new_version_.length()>0)
    {
        const wchar_t* const names[] = { L"File", L"Window", L"Tools", L"Update Available",0 };
        return StringArray ((const wchar_t**) names);
    }
    else
    {
        const wchar_t* const names[] = { L"File", L"Window", L"Tools", 0 };
        return StringArray ((const wchar_t**) names);
    }
}

void EigenMainWindow::menuItemSelected  (int menuItemID, int topLevelMenuIndex)
{
}

EigenTreeItem::EigenTreeItem (EigenLoadComponent *view, const piw::term_t &term): term_(term), view_(view)
{
}

bool EigenTreeItem::select_setup(const char *setup)
{
    piw::term_t ch = term_.arg(1);

    if(ch.arity()>0)
    {
        setOpen(true);

        unsigned n = getNumSubItems();

        for(unsigned i=0;i<n;i++)
        {
            EigenTreeItem *et = (EigenTreeItem *)getSubItem(i);

            if(et->select_setup(setup))
            {
                return true;
            }
        }

        setOpen(false);
    }

    if(term_.arity()>2)
    {
        if(!strcmp(term_.arg(4).value().as_string(),setup))
        {
            setSelected(true,true);
            return true;
        }
    }

    return false;
}

void EigenTreeItem::tree_changed(const piw::term_t &term)
{
    term_ = term;
    clearSubItems();
    treeHasChanged();
    setOpen(false);
    setOpen(true);
}

EigenTreeItem::~EigenTreeItem()
{
}

int EigenTreeItem::getItemWidth() const
{
    return -1;
}

bool EigenTreeItem::mightContainSubItems()
{
    piw::term_t ch = term_.arg(1);

    if(ch.arity()>0)
    {
        return true;
    }

    return false;
}

juce::String EigenTreeItem::getSlot()
{
    if(term_.arity()>2)
    {
        return term_.arg(3).value().as_string();
    }

    return "";
}

void EigenTreeItem::paintItem (Graphics& g, int width, int height)
{
    juce::String s;

    s = term_.arg(0).value().as_string();

    if(term_.arity()>2)
    {
        piw::data_t tag  = term_.arg(2).value();
        if(tag.is_string() && strlen(tag.as_string())>0)
        {
            s+=" ("; s+=String::fromUTF8(tag.as_string()); s+=")";
        }
    }

    if (isSelected()) g.fillAll (Colours::black.withAlpha (0.2f));

    g.setColour (Colours::black);
    g.setFont (height * 0.7f);
    g.drawText (s,4, 0, width - 4, height, Justification::centredLeft, true);
}

void EigenTreeItem::itemOpennessChanged (bool isNowOpen)
{
    if (isNowOpen)
    {
        if (getNumSubItems() == 0)
        {
            piw::term_t ch = term_.arg(1);

            for(unsigned i=0;i<ch.arity();i++)
            {
                addSubItem(new EigenTreeItem(view_,ch.arg(i)));
            }
        }
    }
}

EigenBugComponent::EigenBugComponent(EigenMainWindow *mediator): mediator_(mediator)
{
    setName("Bug");
    pic::msg_t bug_report;

    bug_report << "Explain what happened:\n"
        << "\n\n"
        << "System information:\n"
        << "OS: " << SystemStats::getOperatingSystemName() << " "
        << "64 bit: " << (SystemStats::isOperatingSystem64Bit()?"yes":"no") << "\n"
        << "CPU: " << SystemStats::getCpuVendor() << ", "
        << "Cores: " << SystemStats::getNumCpus() << ", "
        << "Speed: " << SystemStats::getCpuSpeedInMegaherz() << " MHz\n"
        << "Memory: " << SystemStats::getMemorySizeInMegabytes() << " MB\n"
        << "Version: " << pic::release() << "\n" << "\n";

    name_editor()->setText(mediator->backend()->get_username().c_str(),false);
    email_editor()->setText(mediator->backend()->get_email().c_str(),false);
    subject_editor()->setText(mediator->backend()->get_subject().c_str(),false);
    description_editor()->setText(bug_report.str().c_str());

    if(name_editor()->getText().length()==0)
    {
        name_editor()->grabKeyboardFocus();
        return;
    }

    if(email_editor()->getText().length()==0)
    {
        email_editor()->grabKeyboardFocus();
        return;
    }

    if(subject_editor()->getText().length()==0)
    {
        subject_editor()->grabKeyboardFocus();
        return;
    }

    description_editor()->grabKeyboardFocus();
}

EigenBugComponent::~EigenBugComponent()
{
}

void EigenBugComponent::buttonClicked (Button* buttonThatWasClicked)
{
    mediator_->backend()->file_bug(
            std::string(name_editor()->getText().trim().getCharPointer()),
            std::string(email_editor()->getText().trim().getCharPointer()),
            std::string(subject_editor()->getText().trim().getCharPointer()),
            std::string(description_editor()->getText().trim().getCharPointer())
            );

    pic::bgprocess_t(pic::private_exe_dir(),"eigenbugreporter",true).start();

    delete getTopLevelComponent();
}

unsigned EigenSaveComponent::getUserNumber(const juce::String &tag)
{
    juce::String tag2 = tag.trim();

    if(!tag2.startsWith("user "))
    {
        return 0;
    }

    juce::String tag3 = tag.substring(5).trim();
    juce::String tag4 = tag3.removeCharacters("0123456789");

    if(tag3.length()==0 || tag4.length()>0)
    {
        return 0;
    }

    return tag3.getIntValue();
}

EigenSaveComponent::EigenSaveComponent(EigenMainWindow *mediator, const std::string &current): mediator_(mediator), confirm_(0)
{
    setName("Save");
    term_ = mediator->backend()->get_user_setups();
    max_ = 0;
    bool c = false;
    bool cun = false;

    getUserButton()->setToggleState(true,false);
    getErrorLabel()->setVisible(false);

    for(unsigned i=1;i<term_.arity();i++)
    {
        juce::String sltt = term_.arg(i).arg(1).value().as_string();
        juce::String sltt2 = mediator_->backend()->words_to_notes(std::string(sltt.getCharPointer())).c_str();

        getWordsChooser()->addItem(sltt,i);
        getNotesChooser()->addItem(sltt2,i);

        int un = getUserNumber(sltt);

        if(!strcmp(current.c_str(),term_.arg(i).arg(2).value().as_string()))
        {
            getWordsChooser()->setText(sltt);
            c = true;
            if(un) cun=true;
        }

        if(!un)
        {
            continue;
        }

        juce::String usr = sltt;
        if(term_.arg(i).arg(0).value().is_string())
        {
            usr = sltt + " (" + term_.arg(i).arg(0).value().as_string() + ")";
        }
        getUserChooser()->addItem(usr,i);

        if(un>max_) max_=un;
    }

    max_+=1;


    if(!c)
    {
        //buttonClicked(newButton());
        getWordsChooser()->setEnabled(false);
        getNotesChooser()->setEnabled(false);
        getUserChooser()->setEnabled(true);
        getWordsLabel()->setEnabled(false);
        getNotesLabel()->setEnabled(false);
        getUserLabel()->setEnabled(true);
        getDescription()->setText("",false);
    }
    else
    {
        comboBoxChanged(getWordsChooser());
        getNotesLabel()->setEnabled(false);
        getNotesChooser()->setEnabled(false);

        getWordsChooser()->setEnabled(!cun);
        getWordsLabel()->setEnabled(!cun);
        getWordsButton()->setToggleState(!cun,false);

        getUserChooser()->setEnabled(cun);
        getUserLabel()->setEnabled(cun);
        getUserButton()->setToggleState(cun,false);

        juce::String d = mediator_->backend()->get_description(current.c_str()).c_str();
        getDescription()->setText(d,false);
    }
}

EigenSaveComponent::~EigenSaveComponent()
{
    mediator_->show_save_help(false);
    cancel_confirm();
}

void EigenSaveComponent::cancel_confirm()
{
    if(confirm_)
    {
        ((EigenAlertComponent2 *)(confirm_->getContentComponent()))->set_listener(0);
        delete confirm_;
        confirm_=0;
    }
}

void EigenSaveComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    cancel_confirm();

    if(comboBoxThatHasChanged == getUserChooser())
    {
        juce::String t(term_.arg(getUserChooser()->getSelectedId()).arg(1).value().as_string());
        getWordsChooser()->setText(t,false);
        comboBoxChanged(getWordsChooser());
        return;
    }

    if(comboBoxThatHasChanged == getWordsChooser())
    {
        juce::String t = getWordsChooser()->getText().trim();
        int un = getUserNumber(t);

        if(un>0)
        {
            getUserChooser()->setText(t,true);
        }
        else
        {
            getUserChooser()->setText("",true);
        }

        for(unsigned i=1;i<term_.arity();i++)
        {
            juce::String sltt = term_.arg(i).arg(1).value().as_string();
            sltt.trim();

            if(sltt.compare(t)==0)
            {
                if(term_.arg(i).arg(0).value().is_string())
                {
                    juce::String desc = term_.arg(i).arg(0).value().as_string();
                    getSummary()->setText(desc,false);
                }
                else
                {
                    getSummary()->setText("",false);
                }

                juce::String d = mediator_->backend()->get_description(term_.arg(i).arg(2).value().as_string()).c_str();
                getDescription()->setText(d,false);
            }
        }

        getWordsChooser()->setText(t,true);
        std::string n = std::string(mediator_->backend()->words_to_notes(std::string(t.getCharPointer())));
        getErrorLabel()->setVisible(false);

        if(n.length()==0)
        {
            getSaveButton()->setEnabled(false);

            if(t.length()>0 || getNotesChooser()->getText().length()>0)
            {
                getErrorLabel()->setVisible(true);
            }
        }
        else
        {
            getNotesChooser()->setText(n.c_str(),true);
            getSaveButton()->setEnabled(true);
        }

        return;
    }

    if(comboBoxThatHasChanged == getNotesChooser())
    {
        juce::String t = getNotesChooser()->getText();
        std::string n = std::string(mediator_->backend()->notes_to_words(std::string(t.getCharPointer())));
        juce::String tn(n.c_str());
        getWordsChooser()->setText(tn,true);
        comboBoxChanged(getWordsChooser());
        return;
    }

}

void EigenSaveComponent::buttonClicked (Button* buttonThatWasClicked)
{
    cancel_confirm();

    if(buttonThatWasClicked == getUserButton())
    {
        getWordsChooser()->setEnabled(false);
        getNotesChooser()->setEnabled(false);
        getUserChooser()->setEnabled(true);
        getWordsLabel()->setEnabled(false);
        getNotesLabel()->setEnabled(false);
        getUserLabel()->setEnabled(true);
        return;
    }

    if(buttonThatWasClicked == getWordsButton())
    {
        getWordsChooser()->setEnabled(true);
        getNotesChooser()->setEnabled(false);
        getUserChooser()->setEnabled(false);
        getWordsLabel()->setEnabled(true);
        getNotesLabel()->setEnabled(false);
        getUserLabel()->setEnabled(false);
        comboBoxChanged(getWordsChooser());
        return;
    }

    if(buttonThatWasClicked == getNotesButton())
    {
        getWordsChooser()->setEnabled(false);
        getNotesChooser()->setEnabled(true);
        getUserChooser()->setEnabled(false);
        getWordsLabel()->setEnabled(false);
        getNotesLabel()->setEnabled(true);
        getUserLabel()->setEnabled(false);
        comboBoxChanged(getWordsChooser());
        return;
    }

    if(buttonThatWasClicked == getNewButton())
    {
        juce::String slot = juce::String::formatted("user %d",max_);
        getUserChooser()->setText(slot,true);
        getWordsChooser()->setText(slot,true);

        getWordsChooser()->setEnabled(false);
        getNotesChooser()->setEnabled(false);
        getUserChooser()->setEnabled(true);
        getWordsLabel()->setEnabled(false);
        getNotesLabel()->setEnabled(false);
        getUserLabel()->setEnabled(true);

        getUserButton()->setToggleState(true,false);
        getNotesButton()->setToggleState(false,false);
        getWordsButton()->setToggleState(false,false);
        comboBoxChanged(getWordsChooser());
        return;
    }

    if(buttonThatWasClicked == getHelpButton())
    {
        mediator_->show_save_help(true);
        return;
    }

    if(buttonThatWasClicked == getSaveButton())
    {
        juce::String slot = getWordsChooser()->getText().trim();

        if(slot.length()==0)
        {
            return;
        }

        std::string old_setup = mediator_->backend()->get_setup_slot(slot.toUTF8());
        getSaveButton()->setEnabled(false);

        if(old_setup.length()==0)
        {
            alert_ok();
            return;
        }

        juce::String msg = "This will overwrite an existing setup named\n\n";
        msg += slot;

        if(strcmp(old_setup.c_str(),"none"))
        {
            msg += "\n\nand tagged\n\n";
            msg += juce::String(old_setup.c_str());
        }

        confirm_ = mediator_->alert2("Overwrite Setup","Overwrite Setup",msg,this);
    }
}

void EigenSaveComponent::alert_ok()
{
    EigenMainWindow *m = mediator_;
    juce::String slot = getWordsChooser()->getText().trim();
    juce::String name = getSummary()->getText().trim();
    juce::String desc = getDescription()->getText().trim();
    m->backend()->save_setup(slot.toUTF8(),name.toUTF8(),desc.toUTF8(),getDefaultButton()->getToggleState());
    confirm_ = 0;
    delete getTopLevelComponent();
}

void EigenSaveComponent::alert_cancel()
{
    confirm_ = 0;
    getSaveButton()->setEnabled(true);
}

EigenEditComponent::EigenEditComponent(EigenMainWindow *mediator, const std::string &current): mediator_(mediator)
{
    term_ = mediator->backend()->get_user_setups();

    for(unsigned i=1;i<term_.arity();i++)
    {
        if(!strcmp(current.c_str(),term_.arg(i).arg(2).value().as_string()))
        {
            orig_=term_.arg(i).arg(2).value().as_string();

            juce::String slot=term_.arg(i).arg(1).value().as_string();
            getSlot()->setText(slot,false);

            if(term_.arg(i).arg(0).value().is_string())
            {
                juce::String desc=term_.arg(i).arg(0).value().as_string();
                getSummary()->setText(desc,false);
            }
            else
            {
                getSummary()->setText("",false);
            }

            break;
        }
    }

    juce::String d=mediator_->backend()->get_description(current.c_str()).c_str();
    getDescription()->setText(d,false);
}

EigenEditComponent::~EigenEditComponent()
{
    mediator_->show_save_help(false);
}

void EigenEditComponent::buttonClicked (Button* buttonThatWasClicked)
{
    if(buttonThatWasClicked == helpButton())
    {
        mediator_->show_save_help(true);
        return;
    }

    getButton()->setEnabled(false);

    juce::String slot = getSlot()->getText().trim();
    juce::String name = getSummary()->getText().trim();
    juce::String desc = getDescription()->getText().trim();
    mediator_->backend()->edit_setup(orig_.c_str(),slot.toUTF8(),name.toUTF8(),desc.toUTF8());

    delete getTopLevelComponent();
}

bool EigenMainWindow::select_setup(const char *setup)
{
    return component_->select_setup(setup);
}

EigenDialog::EigenDialog(EigenMainWindow *main, Component *content, int w, int h, int mw, int mh, int xw, int xh, Component *position, int buttons): DocumentWindow(content->getName(), Colours::black, buttons, true), main_(main), component_(content)
{
    setContentOwned(content, true);

    setUsingNativeTitleBar(true);

    setSize(w,h);
    
    if(mw>0)
    {
        setResizeLimits(mw, mh, xw, xh);
        setResizable (true, true);
    }
    else
    {
        setResizeLimits(1,1,w,h);
        setResizable (false, false);
    }
    
    if(position)
    {
        centreAroundComponent (position, getWidth(), getHeight());
    }
    else
    {
        setCentrePosition (160,80);
    }

    setVisible (true);
    toFront(true);
    addKeyListener(this);

}

void EigenDialog::handleMessage (const Message &m)
{
    EigenDMessage *msg = (EigenDMessage *)&m;
    if(msg->type_ == messageConfirm)
    {
        EigenDialogContent *c = dynamic_cast<EigenDialogContent *>(component_);

        if(c)
        {
            c->returnKeyPressed();
        }
    }
    else
    {
        closeButtonPressed();
    }
}

bool EigenDialog::keyPressed (const KeyPress& key, Component* originatingComponent)
{
    if(key.getKeyCode() == KeyPress::escapeKey)
    {
        postMessage(new EigenDMessage(messageEscape));
        return true;
    }

    if(key.getKeyCode() == KeyPress::returnKey)
    {
        EigenDialogContent *c = dynamic_cast<EigenDialogContent *>(component_);

        if(c)
        {
            postMessage(new EigenDMessage(messageConfirm));
        }
    }

    return false;
}

bool EigenDialog::keyStateChanged (bool isKeyDown, Component* originatingComponent)
{
    return false;
}

void EigenDialog::closeButtonPressed()
{
    delete this;
}

EigenDialog::~EigenDialog()
{
    main_->windowClosed(this);
    main_->dialog_dead(this);
}

EigenD::EigenD(): main_window_ (0), python_(0), logfile_(0), lockfile_("EigenD")
{
}

EigenD::~EigenD()
{
}

void EigenD::initialise (const String& commandLine)
{
    pic_set_interrupt();
    pic_mlock_code();
    pic_init_time();

    if(!lockfile_.lock())
    {
        ModalComponent mc;
        mc.getTitle()->setText("eigenD is already running",false);
        mc.getText()->setText("Another instance of eigenD is already running.  You have to close it before running eigenD again.",false);
        DialogWindow::showModalDialog("Another eigenD Running",&mc,0,Colour(0xffababab),true,false);
        quit();
        return;
    }

    printf("release root: %s\n",pic::release_root_dir().c_str());

    pic::bgprocess_t(pic::private_exe_dir(),"eigenbugreporter",true).start();

    pic::f_string_t primary_logger = pic::f_string_t::method(this,&EigenD::log);
    pic::f_string_t eigend_logger = EigenLogger::create("eigend",primary_logger);

    ejuce::Application::initialise(commandLine,eigend_logger,false,true);
    LookAndFeel::setDefaultLookAndFeel(new ejuce::EJuceLookandFeel);
    ApplicationCommandManager *manager = new ApplicationCommandManager();

    python_ = new epython::PythonInterface();
    context_ = scaffold()->context("main",pic::status_t(),eigend_logger,"eigend");

    piw::tsd_setcontext(context_.entity());
    python_->py_startup();

    if(python_->init_python("app_eigend2.backend","main"))
    {
        eigend::c2p_t *backend = (eigend::c2p_t *)python_->mediator();
        if(backend)
        {
            backend->set_args(commandLine.toUTF8());
            std::string logfile = backend->get_logfile();

            if(logfile.length()>0)
            {
                logfile_ = pic::fopen(logfile,"w");
            }

            main_window_ = new EigenMainWindow(manager,scaffold(),backend,primary_logger);
        }
        else
        {
            juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon, "An unexpected error occurred ...", python_->last_error().c_str());
        }
    }
    else
    {
        juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon, "An unexpected error occurred ...", python_->last_error().c_str());
    }

}

void EigenD::systemRequestedQuit()
{
    if(main_window_ != 0)
    {
        EigenMainWindow *w = main_window_;
        main_window_ = 0;

        if(!w->do_quit())
        {
            main_window_ = w;
            return;
        }

        delete w;
    }

    juce::MessageManager::getInstance()->runDispatchLoopUntil(500);
    cleanup();
    ejuce::Application::quit();
}

void EigenD::shutdown()
{
    ejuce::Application::shutdown();
}

const String EigenD::getApplicationName()
{
    return "eigenD";
}

const String EigenD::getApplicationVersion()
{
    return pic::release().c_str();
}

bool EigenD::moreThanOneInstanceAllowed()
{
    return true;
}

void EigenD::log(const char *msg)
{
    if(logfile_)
    {
        fprintf(logfile_,"%s\n",msg);
        fflush(logfile_);
    }
    else
    {
        printf("%s\n",msg);
        fflush(stdout);
    }
}

void EigenD::anotherInstanceStarted (const String& commandLine)
{
}

EigenStatusComponent::EigenStatusComponent(EigenMainWindow *mediator): mediator_(mediator)
{
    setName("System Usage Meter");
}

EigenStatusComponent::~EigenStatusComponent()
{
}


EigenAboutComponent::EigenAboutComponent(EigenMainWindow *mediator): mediator_(mediator)
{
    setName("About");
}

EigenAboutComponent::~EigenAboutComponent()
{
}

void EigenMainWindow::windowClosed(Component *window)
{
    if(window==about_) { about_=0; }
    if(window==status_) { status_=0; }
    if(window==saving_) { saving_=0; }
    if(window==editing_) { editing_=0; }
    if(window==bug_) { bug_=0; }
    if(window==progress_) { progress_=0; }
    if(window==help_) { help_=0; }
}

pic::f_string_t EigenMainWindow::make_logger(const char *prefix)
{
    JUCE_AUTORELEASEPOOL
        return EigenLogger::create(prefix,logger_);
}

void EigenMainWindow::save()
{
    if(!backend_->save_current_setup())
    {
        save_dialog(current_setup_);
    }
}

void EigenMainWindow::save_dialog()
{
    save_dialog(current_setup_);
}

void EigenMainWindow::save_dialog(const std::string &current)
{
    if(!saving_)
    {
        saving_ = new EigenDialog(this,new EigenSaveComponent(this,current),600,600,600,600,2000,2000,this);
    }
}

void EigenMainWindow::edit_dialog(const std::string &current)
{
    if(!editing_)
    {
        editing_ = new EigenDialog(this,new EigenEditComponent(this,current),600,500,600,500,2000,2000,this);
    }
}

void EigenMainWindow::alert_dialog(const char *klass, const char *label, const char *text)
{
    JUCE_AUTORELEASEPOOL
    alert1(klass,label,text);
}

EigenDialog *EigenMainWindow::alert1(const String &klass, const String &label, const String &text,bool left)
{
    std::string cklass = std::string(klass.getCharPointer());

    if(ignores_.getBoolValue(juce::String("ignores.")+klass, false))
    {
        return 0;
    }

    EigenAlertComponent1 *c = new EigenAlertComponent1(this,klass,label,text);

    if(left)
    {
        c->set_left();
    }

    EigenDialog *e = new EigenDialog(this,c,400,600,400,600,2000,2000,this);

    std::map<std::string,EigenDialog *>::iterator i;

    if((i=alert_dialogs_.find(cklass)) != alert_dialogs_.end())
    {
        e->setBounds(i->second->getBounds());
        delete i->second;
    }

    alert_dialogs_.insert(std::make_pair(cklass,e));
    return e;
}

EigenDialog *EigenMainWindow::alert2(const String &klass, const String &label, const String &text, EigenAlertDelegate *l)
{
    std::string cklass = std::string(klass.getCharPointer());

    if(ignores_.getBoolValue(juce::String("ignores.")+klass, false))
    {
        l->alert_ok();
        return 0;
    }

    EigenAlertComponent2 *c = new EigenAlertComponent2(this,klass,label,text,l);
    EigenDialog *e = new EigenDialog(this,c,400,600,400,600,2000,2000,this);

    std::map<std::string,EigenDialog *>::iterator i;

    if((i=alert_dialogs_.find(cklass)) != alert_dialogs_.end())
    {
        delete i->second;
    }

    alert_dialogs_.insert(std::make_pair(cklass,e));
    return e;
}

void EigenMainWindow::info_dialog(const char *klass, const char *label, const char *text)
{
    JUCE_AUTORELEASEPOOL
    info(klass,label,text);
}

EigenDialog *EigenMainWindow::info(const String &klass, const String &label, const String &text)
{
    std::string cklass = std::string(klass.getCharPointer());

    EigenInfoComponent *c = new EigenInfoComponent(this,klass,label,text);
    EigenDialog *e = new EigenDialog(this,c,400,400,400,400,2000,2000,this);

    std::map<std::string,EigenDialog *>::iterator i;

    if((i=info_dialogs_.find(cklass)) != info_dialogs_.end())
    {
        delete i->second;
    }

    info_dialogs_.insert(std::make_pair(cklass,e));
    return e;
}

void EigenMainWindow::dialog_dead(EigenDialog *e)
{
    std::map<std::string,EigenDialog *>::iterator i;

    if((i=alert_dialogs_.find(std::string(e->getName().toUTF8()))) != alert_dialogs_.end())
    {
        if(e==i->second)
        {
            alert_dialogs_.erase(i);
        }
    }

    if((i=info_dialogs_.find(std::string(e->getName().toUTF8()))) != info_dialogs_.end())
    {
        if(e==i->second)
        {
            info_dialogs_.erase(i);
        }
    }
}

EigenAlertComponent2::EigenAlertComponent2(EigenMainWindow *main,const String &klass,const String &label, const String &text, EigenAlertDelegate *listener): main_(main), listener_(listener)
{
    setName(klass);
    set_title(label);
    set_text(text);
}

EigenAlertComponent2::~EigenAlertComponent2()
{
    if(listener_)
    {
        listener_->alert_cancel();
    }
}

void EigenAlertComponent2::set_listener(EigenAlertDelegate *l)
{
    listener_ = l;
}

void EigenAlertComponent2::buttonClicked (Button* b)
{
    if(get_toggle_button()->getToggleState())
    {
        main_->ignore_klass(getName());
    }

    if(b == get_ok_button())
    {
        EigenAlertDelegate *listener = listener_;
        listener_ = 0;

        delete getTopLevelComponent();

        if(listener)
        {
            listener->alert_ok();
        }
    }
    else
    {
        delete getTopLevelComponent();
    }

}

EigenHelpComponent::EigenHelpComponent(EigenMainWindow *main,const String &klass,const String &label, const String &text): main_(main)
{
    setName(klass);

    set_title(label);
    set_text(text);
}

EigenHelpComponent::~EigenHelpComponent()
{
}

void EigenHelpComponent::buttonClicked (Button* buttonThatWasClicked)
{
    delete getTopLevelComponent();
}

EigenAlertComponent1::EigenAlertComponent1(EigenMainWindow *main,const String &klass,const String &label, const String &text): main_(main)
{
    setName(klass);

    set_title(label);
    set_text(text);
}

EigenAlertComponent1::~EigenAlertComponent1()
{
}

void EigenAlertComponent1::buttonClicked (Button* buttonThatWasClicked)
{
    if(get_toggle_button()->getToggleState())
    {
        main_->ignore_klass(getName());
    }

    delete getTopLevelComponent();
}

EigenInfoComponent::EigenInfoComponent(EigenMainWindow *main,const String &klass,const String &label, const String &text): main_(main)
{
    setName(klass);

    set_title(label);
    set_text(text);
}

EigenInfoComponent::~EigenInfoComponent()
{
}

void EigenInfoComponent::buttonClicked (Button* buttonThatWasClicked)
{
    delete getTopLevelComponent();
}

EigenLogger::EigenLogger(const char *prefix, const pic::f_string_t &logger): prefix_(prefix), logger_(logger)
{
}

EigenLogger::EigenLogger(const EigenLogger &l): prefix_(l.prefix_), logger_(l.logger_)
{
}

EigenLogger &EigenLogger::operator=(const EigenLogger &l)
{
    prefix_=l.prefix_;
    logger_=l.logger_;
    return *this;
}

bool EigenLogger::operator==(const EigenLogger &l) const
{
    return (logger_==l.logger_) && (prefix_.compare(l.prefix_)==0);
}

pic::f_string_t EigenLogger::create(const char *prefix, const pic::f_string_t &logger)
{
    return pic::f_string_t::callable(EigenLogger(prefix,logger));
}

void EigenLogger::operator()(const char *msg) const
{
    juce::String buffer(prefix_);
    buffer += ": "; buffer+=msg;
    logger_(std::string(buffer.getCharPointer()).c_str());
}

void EigenMainWindow::ignore_klass(const juce::String &klass)
{
    std::string cklass = std::string(klass.getCharPointer());
    ignores_.setValue(juce::String("ignores.")+klass, true);
}

void EigenD::handleWinch(const std::string &msg)
{
    JUCE_AUTORELEASEPOOL

    if(!main_window_)
    {
        return;
    }

    if(msg.length()==0)
    {
        main_window_->menuItemsChanged();
        return;
    }

    juce::String jmsg = juce::String::fromUTF8(msg.c_str());
    int c1 = jmsg.indexOf(0,":");

    if(c1<0)
    {
        return;
    }

    int c2 = jmsg.indexOf(c1+1,":");

    if(c2<0)
    {
        return;
    }

    juce::String alert_klass = jmsg.substring(0,c1);
    juce::String alert_title = jmsg.substring(c1+1,c2);
    juce::String alert_msg = jmsg.substring(c2+1);
    
    main_window_->alert1(alert_klass,alert_title,alert_msg);
}

START_JUCE_APPLICATION (EigenD)
