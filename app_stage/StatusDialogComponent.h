/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

#ifndef __JUCE_HEADER_48392B2CED10A6A2__
#define __JUCE_HEADER_48392B2CED10A6A2__

//[Headers]     -- You can add your own extra header files here --
/*
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

#include "Network.h"
#include "juce.h"
#include "ProgressBarSync.h"

class ConnectionManager;
class StatusDialogComponent;

class StatusListModel: public ListBoxModel
{
    public:
        StatusListModel();
        void setConnectionManager(ConnectionManager *m);
        ConnectionManager *getConnectionManager() { return manager_; }
        int getNumRows();
        void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected);
        void returnKeyPressed(int lastRowSelected);
    private:
        ConnectionManager *manager_;
};

class StatusTimer: public Timer
{
    public:
        StatusTimer(StatusDialogComponent *);
        ~StatusTimer();
        void timerCallback();

    private:
        StatusDialogComponent *component_;

};

//[/Headers]

#include "DialogButtonComponent.h"


//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class StatusDialogComponent  : public Component,
                               public ComponentBoundsConstrainer,
                               public MessageListener,
                               public ButtonListener
{
public:
    //==============================================================================
    StatusDialogComponent ();
    ~StatusDialogComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void setProgress(const double progress);
    void setTitle(const String title);
    void setStatusMessage(const String statusMessage);
    void showQuitButton(bool shouldBeVisible);
    void showConnectButton(bool shouldBeVisible);
	void showProgressBar(bool shouldBeVisible);
    void showHostList(bool shouldBeVisible);
    void showStatusMessage(bool shouldBeVisible);
    void showAutoToggleButton(bool shouldBeVisible);
    void setUpPosition(Component *parent, int width, int height);
    ProgressBarSync* getProgressBar() { return progressBar_; }
    ListBox* getHostList() { return hostList_; }
    DialogButtonComponent* getConnectButton() { return connectButton; }
    ToggleButton* getAutoToggleButton() { return autoToggleButton; }
    void setConnectionManager(ConnectionManager* model);
    void handleMessage(const Message &message);
    void updateContent();
    void timerCallback();
    void setDesiredService(const String &name);
    //void visibilityChanged();
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    double progress_;
    ProgressBarSync *progressBar_;
    ListBox *hostList_;
    StatusListModel *model_;
    StatusTimer *timer_;
    String desired_service_;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Label> titleLabel;
    ScopedPointer<Label> statusMessageLabel;
    ScopedPointer<DialogButtonComponent> quitButton;
    ScopedPointer<DialogButtonComponent> connectButton;
    ScopedPointer<ToggleButton> autoToggleButton;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatusDialogComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_48392B2CED10A6A2__
