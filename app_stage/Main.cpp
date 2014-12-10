/*
  ==============================================================================

  Eigenlabs Stage Application

  ==============================================================================

 Copyright 2010-2014 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#include "Main.h"

#ifdef _WIN32
#include "guicon.h"
#endif

//==============================================================================
/** This is the application object that is started up when Juce starts. It handles
    the initialisation and shutdown of the whole application.
*/
class StageApplication : public JUCEApplication
{
    /* Important! NEVER embed objects directly inside your JUCEApplication class! Use
       ONLY pointers to objects, which you should create during the initialise() method
       (NOT in the constructor!) and delete in the shutdown() method (NOT in the
       destructor!)

       This is because the application object gets created before Juce has been properly
       initialised, so any embedded objects would also get constructed too soon.
   */
    StageWindow* stageWindow;
    // look and feel
    EigenLookAndFeel* eigenLookAndFeel_;


public:
    //==============================================================================
    StageApplication()
        : stageWindow (0)
    {
        // NEVER do anything in here that could involve any Juce function being called
        // - leave all your startup tasks until the initialise() method.
    }

    ~StageApplication()
    {
        // Your shutdown() method should already have done all the things necessary to
        // clean up this app object, so you should never need to put anything in
        // the destructor.

        // Making any Juce calls in here could be very dangerous...
    }

    //==============================================================================
    void initialise (const String& commandLine)
    {
#ifdef _WIN32
#ifdef _DEBUG
		RedirectIOToConsole();
#endif // _DEBUG
#endif // _WIN32
        // set up custom look and feel
        eigenLookAndFeel_ = new EigenLookAndFeel();
        LookAndFeel::setDefaultLookAndFeel(eigenLookAndFeel_);
        eigenLookAndFeel_->setHasRedHatching(true);
        
        // set look and feel colours
        eigenLookAndFeel_->setColour(PopupMenu::highlightedBackgroundColourId, Colour(0xff387fd7));
        
        eigenLookAndFeel_->setColour(TextEditor::highlightColourId, Colour(0xffcccccc));
        eigenLookAndFeel_->setColour(TextEditor::focusedOutlineColourId, Colour(0xff777777));
        eigenLookAndFeel_->setColour(TextEditor::outlineColourId, Colour(0xff999999));
        
        eigenLookAndFeel_->setColour(ComboBox::buttonColourId, Colour(0xffcccccc));
        eigenLookAndFeel_->setColour(ComboBox::outlineColourId, Colour(0xff777777));
        eigenLookAndFeel_->setColour(TextButton::buttonColourId, Colour(0xff777777));

        eigenLookAndFeel_->setColour(ProgressBar::foregroundColourId, Colour(0xff777777));
        
        eigenLookAndFeel_->setColour(ListBox::backgroundColourId, Colour(0xffe9e9e9));

    //    eigenLookAndFeel_->setDefaultSansSerifTypefaceName("Lucida Grande");
    //    eigenLookAndFeel_->setDefaultSansSerifTypefaceName("Verdana");
    //    eigenLookAndFeel_->setDefaultSansSerifTypefaceName("Helvetica");

        
        // just create the main window...
        stageWindow = new StageWindow();

        /*  ..and now return, which will fall into to the main event
            dispatch loop, and this will run until something calls
            JUCEAppliction::quit().

            In this case, JUCEAppliction::quit() will be called by the
            hello world window being clicked.
        */
    }

    void shutdown()
    {
        // clear up..

        if (stageWindow != 0)
            deleteAndZero(stageWindow);

        deleteAndZero(eigenLookAndFeel_);
    }
    
#if STAGE_BUILD==IOS
    void suspended()
    {
        stageWindow->suspend();
    }
    
    void resumed()
    {
        stageWindow->resume();
    }
#endif // STAGE_BUILD==IOS

    //==============================================================================
    const String getApplicationName()
    {
        return "Eigenlabs Stage";
    }

    const String getApplicationVersion()
    {
#if STAGE_BUILD==DESKTOP
        return "2.1.0";
#else
        return "1.0.3";
#endif
    }

    bool moreThanOneInstanceAllowed()
    {
        return false;
    }

    void anotherInstanceStarted (const String& commandLine)
    {
    }
};



StageWindow::StageWindow(): DocumentWindow ("Stage", Colours::lightgrey, DocumentWindow::allButtons, true)
{
    commandManager = new ApplicationCommandManager();
    
    // Create an instance of our main content component, and add it
    // to our window.
    MainComponent* const contentComponent = new MainComponent();
    contentComponent->setMainWindow(this);
    
    // sets the main content component for the window to be this tabbed
    // panel. This will be deleted when the window is deleted.
    setContentOwned(contentComponent, true);

    // set title bar
    setUsingNativeTitleBar(true);
    setTitleBarHeight(0);
    
#if STAGE_BUILD==DESKTOP
    commandManager->registerAllCommandsForTarget (contentComponent);
    commandManager->registerAllCommandsForTarget (JUCEApplication::getInstance());
    
    // this lets the command manager use keypresses that arrive in our window to send
    // out commands
    addKeyListener(commandManager->getKeyMappings());
    
    // this tells the DocumentWindow to automatically create and manage a MenuBarComponent
    // which uses our contentComp as its MenuBarModel
    MouseEvent::setDoubleClickTimeout(200);
#if JUCE_MAC
    MenuBarModel::setMacMainMenu (contentComponent);
#else
    setMenuBar(contentComponent);
#endif // JUCE_MAC
    
    // tells our menu bar model that it should watch this command manager for
    // changes, and send change messages accordingly.
    contentComponent->setApplicationCommandManagerToWatch(commandManager);

    // resizable window
    setResizable(true, true);
    
    // set minimum size
    ComponentBoundsConstrainer* bounds = getConstrainer();
    bounds->setMinimumSize(500, 256);
    centreWithSize(getWidth(), getHeight());
    
    setBroughtToFrontOnMouseClick(true);
#else
    setFullScreen(true);
#endif // STAGE_BUILD==DESKTOP
        
    setVisible (true);
    
    contentComponent->initializeModel();
    contentComponent->getXMLRPCManager()->disableBrowser(false);
}

StageWindow::~StageWindow()
{
    // force exit full screen
    Desktop::getInstance().setKioskModeComponent (0);
    setUsingNativeTitleBar(true);
    setResizable(true, true);
    
    dynamic_cast<MainComponent*>(getContentComponent())->shutdown();

    // because we've set the content comp to be used as our menu bar model, we
    // have to switch this off before deleting the content comp..
    setMenuBar (0);
    
#if JUCE_MAC  // ..and also the main bar if we're using that on a Mac...
    MenuBarModel::setMacMainMenu (0);
#endif
  
	delete getContentComponent();

    delete commandManager;
        
}

void StageWindow::closeButtonPressed()
{
    // When the user presses the close button, we'll tell the app to quit. This
    // window will be deleted by our StageApplication::shutdown() method
    //
    //dynamic_cast<MainComponent*>(getContentComponent())->shutdown();
    
    JUCEApplication::quit();
}

void StageWindow::activeWindowStatusChanged()
{
    // TODO: on windows enable the menu bar when the main window is active
    
    
#if STAGE_BUILD==DESKTOP
    MainComponent* contentComponent = dynamic_cast<MainComponent*> (getContentComponent());

    Message message;

    if(contentComponent)
        //contentComponent->postMessage(&message);
        contentComponent->bringAgentViewToFront();
#endif // STAGE_BUILD==DESKTOP
    
}

void StageWindow::broughtToFront()
{
#if STAGE_BUILD==DESKTOP
    MainComponent* contentComponent = dynamic_cast<MainComponent*> (getContentComponent());
    
    if(contentComponent)
        contentComponent->bringAgentViewToFront();
#endif // STAGE_BUILD==DESKTOP

}

void StageWindow::suspend()
{
#if STAGE_BUILD==IOS
    MainComponent* contentComponent = dynamic_cast<MainComponent*> (getContentComponent());
    
    if(contentComponent)
        contentComponent->getXMLRPCManager()->suspend();
#endif // STAGE_BUILD==IOS
}

void StageWindow::resume()
{
#if STAGE_BUILD==IOS
    MainComponent* contentComponent = dynamic_cast<MainComponent*> (getContentComponent());
    
    if(contentComponent)
        contentComponent->getXMLRPCManager()->resume();
#endif // STAGE_BUILD==IOS
}

void StageWindow::handleMessage(const Message& message)
{
    closeButtonPressed();
}

//==============================================================================
// This macro creates the application's main() function..
START_JUCE_APPLICATION (StageApplication)
