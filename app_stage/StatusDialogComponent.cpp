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

//[Headers] You can add your own extra header files here...
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
//[/Headers]

#include "StatusDialogComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
StatusDialogComponent::StatusDialogComponent ()
{
    setName ("StatusDialog");
    addAndMakeVisible (titleLabel = new Label ("title label",
                                               TRANS("Status")));
    titleLabel->setFont (Font (17.20f, Font::bold));
    titleLabel->setJustificationType (Justification::centredLeft);
    titleLabel->setEditable (false, false, false);
    titleLabel->setColour (Label::textColourId, Colour (0xffc8c8c8));
    titleLabel->setColour (TextEditor::textColourId, Colours::black);
    titleLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (statusMessageLabel = new Label ("status message label",
                                                       TRANS("Status message")));
    statusMessageLabel->setFont (Font (15.00f, Font::plain));
    statusMessageLabel->setJustificationType (Justification::centred);
    statusMessageLabel->setEditable (false, false, false);
    statusMessageLabel->setColour (TextEditor::textColourId, Colours::black);
    statusMessageLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (quitButton = new DialogButtonComponent());
    addAndMakeVisible (connectButton = new DialogButtonComponent());
    addAndMakeVisible (autoToggleButton = new ToggleButton ("auto toggle button"));
    autoToggleButton->setButtonText (TRANS("auto connect when single eigenD"));
    autoToggleButton->addListener (this);
    autoToggleButton->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));


    //[UserPreSize]
    addAndMakeVisible(progressBar_ = new ProgressBarSync (progress_));
    addChildComponent(hostList_ = new ListBox("host list box",0));
    hostList_->setRowHeight(20);
    hostList_->setModel(model_ = new StatusListModel());
    timer_ = new StatusTimer(this);
    //[/UserPreSize]

    setSize (320, 224);


    //[Constructor] You can add your own custom stuff here..
    statusMessageLabel->setMinimumHorizontalScale(1);
    progressBar_->setPercentageDisplay(true);

    quitButton->addListener(this);
    quitButton->setText("quit");

    connectButton->addListener(this);
    connectButton->setText("connect");

    // keyboard shortcuts for buttons
    const KeyPress returnKey(KeyPress::returnKey, 0, 0);
    const KeyPress escapeKey(KeyPress::escapeKey, 0, 0);
    connectButton->addShortcut(returnKey);
    quitButton->addShortcut(escapeKey);

    hostList_->setWantsKeyboardFocus(true);
    hostList_->setExplicitFocusOrder(1);
    titleLabel->setWantsKeyboardFocus(false);
    progressBar_->setWantsKeyboardFocus(false);
    statusMessageLabel->setWantsKeyboardFocus(false);
    quitButton->setWantsKeyboardFocus(false);
    autoToggleButton->setWantsKeyboardFocus(false);
    updateContent();
    //[/Constructor]
}

StatusDialogComponent::~StatusDialogComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..

    timer_->stopTimer();

    if(model_->getConnectionManager())
    {
        model_->getConnectionManager()->clearMessageListener(this);
    }

    //[/Destructor_pre]

    titleLabel = nullptr;
    statusMessageLabel = nullptr;
    quitButton = nullptr;
    connectButton = nullptr;
    autoToggleButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    deleteAndZero(progressBar_);
    deleteAndZero(hostList_);
    deleteAndZero(model_);
    deleteAndZero(timer_);
    //[/Destructor]
}

//==============================================================================
void StatusDialogComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.setColour (Colour (0xcf000000));
    g.fillRoundedRectangle (3.0f, 3.0f, static_cast<float> (getWidth() - 6), static_cast<float> (getHeight() - 6), 5.000f);

    g.setColour (Colour (0xffa3a3a3));
    g.drawRoundedRectangle (3.0f, 3.0f, static_cast<float> (getWidth() - 6), static_cast<float> (getHeight() - 6), 5.000f, 2.700f);

    g.setColour (Colour (0xffe9e9e9));
    g.fillRoundedRectangle (26.0f, 50.0f, static_cast<float> (getWidth() - 52), static_cast<float> (getHeight() - 132), 3.000f);

    g.setColour (Colour (0xffa3a3a3));
    g.drawRoundedRectangle (26.0f, 50.0f, static_cast<float> (getWidth() - 52), static_cast<float> (getHeight() - 132), 3.000f, 2.700f);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void StatusDialogComponent::resized()
{
    titleLabel->setBounds (12, 12, getWidth() - 20, 24);
    statusMessageLabel->setBounds (32, 48, getWidth() - 64, getHeight() - 128);
    quitButton->setBounds (getWidth() - 216, getHeight() - 40, 98, 24);
    connectButton->setBounds (getWidth() - 112, getHeight() - 40, 98, 24);
    autoToggleButton->setBounds (24, getHeight() - 72, getWidth() - 48, 24);
    //[UserResized] Add your own custom resize handling here..
    progressBar_->setBounds(24, getHeight() - 64,  getWidth() - 48, 18);
    hostList_->setBounds(32, 56, getWidth() - 64, getHeight() - 144);
    //[/UserResized]
}

void StatusDialogComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == autoToggleButton)
    {
        //[UserButtonCode_autoToggleButton] -- add your button handler code here..
        // connectionManager_->setConnectSingleHost(autoToggleButton->getToggleState());

        if(hostList_->getModel()->getNumRows()==1 && autoToggleButton->getToggleState())
        {
            if(!timer_->getTimerInterval())
            {
                std::cout << "starting countdown\n";
                timer_->startTimer(3000);
            }
        }
        else
        {
            timer_->stopTimer();
        }

        //[/UserButtonCode_autoToggleButton]
    }

    //[UserbuttonClicked_Post]
    if (buttonThatWasClicked == quitButton)
    {
        exitModalState(-1);
    }

    if (buttonThatWasClicked == connectButton)
    {
        // attempt to connect to the host
        exitModalState(hostList_->getLastRowSelected());
    }
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void StatusDialogComponent::timerCallback()
{
    std::cout << "timer ping..\n";
    if(hostList_->getModel()->getNumRows()==1 && autoToggleButton->getToggleState())
    {
        exitModalState(0);
    }
}

void StatusDialogComponent::setProgress(const double progress)
{
    progress_ = progress;
}

void StatusDialogComponent::setTitle(const String title)
{
    titleLabel->setText(title, dontSendNotification);
}

void StatusDialogComponent::setStatusMessage(const String statusMessage)
{
    statusMessageLabel->setText(statusMessage, dontSendNotification);
}

void StatusDialogComponent::showQuitButton(bool shouldBeVisible)
{
    quitButton->setVisible(shouldBeVisible);
}

void StatusDialogComponent::showConnectButton(bool shouldBeVisible)
{
    connectButton->setVisible(shouldBeVisible);
}

void StatusDialogComponent::showProgressBar(bool shouldBeVisible)
{
	progressBar_->setVisible(shouldBeVisible);
}

void StatusDialogComponent::showHostList(bool shouldBeVisible)
{
	hostList_->setVisible(shouldBeVisible);
}

void StatusDialogComponent::showStatusMessage(bool shouldBeVisible)
{
	statusMessageLabel->setVisible(shouldBeVisible);
}

void StatusDialogComponent::showAutoToggleButton(bool shouldBeVisible)
{
    autoToggleButton->setVisible(shouldBeVisible);
}

void StatusDialogComponent::setConnectionManager(ConnectionManager *manager)
{
    model_->setConnectionManager(manager);
    manager->setMessageListener(this);
}

void StatusDialogComponent::handleMessage(const Message &m)
{
    updateContent();
}

void StatusDialogComponent::updateContent()
{
    hostList_->updateContent();

    if(!desired_service_.isEmpty())
    {
        int i;

        for(i=0;i<model_->getNumRows();i++)
        {
            String service = model_->getConnectionManager()->getHost(i).name;

            if(service == desired_service_)
            {
                exitModalState(i);
            }
        }
    }

    if(hostList_->getModel()->getNumRows()>0)
    {
        showHostList(true);
        showStatusMessage(false);
        connectButton->setEnabled(true);

        if(hostList_->getLastRowSelected()==-1)
        {
            hostList_->selectRow(0);
        }

        hostList_->grabKeyboardFocus();
    }
    else
    {
        showHostList(false);
        showStatusMessage(true);
        connectButton->setEnabled(false);
    }

    if(hostList_->getModel()->getNumRows()==1 && autoToggleButton->getToggleState())
    {
        if(!timer_->getTimerInterval())
        {
            std::cout << "starting countdown\n";
            timer_->startTimer(3000);
        }
    }
    else
    {
        timer_->stopTimer();
    }
}

StatusListModel::StatusListModel(): manager_(0)
{
}

void StatusListModel::setConnectionManager(ConnectionManager *m)
{
    manager_ = m;
}

int StatusListModel::getNumRows()
{
    if(!manager_)
    {
        return 0;
    }

    return manager_->getNumHosts();
}

void StatusListModel::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll (Colour(0xbb387fd7));

    g.setColour (Colours::black);
    g.setFont (height * 0.7f);

    String rowText;

    if(manager_)
    {
        rowText = manager_->getHost(rowNumber).name;
    }

    g.drawText (rowText, 5, 0, width, height, Justification::centredLeft, true);

}

void StatusListModel::returnKeyPressed(int lastRowSelected)
{
}

void StatusDialogComponent::setUpPosition(Component *parent, int width, int height)
{
    int x = parent->getScreenX()+parent->getWidth()/2-width/2;
    int y = parent->getScreenY()+parent->getHeight()/2-height/2;
    setBounds(x,y,width,height);
}

StatusTimer::StatusTimer(StatusDialogComponent *c): component_(c)
{
}

StatusTimer::~StatusTimer()
{
    stopTimer();
}

void StatusTimer::timerCallback()
{
    component_->timerCallback();
}

void StatusDialogComponent::setDesiredService(const String &service)
{
    desired_service_ = service;
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="StatusDialogComponent" componentName="StatusDialog"
                 parentClasses="public Component, public ComponentBoundsConstrainer, public MessageListener"
                 constructorParams="" variableInitialisers="" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="1" initialWidth="320"
                 initialHeight="224">
  <BACKGROUND backgroundColour="0">
    <ROUNDRECT pos="3 3 6M 6M" cornerSize="5" fill="solid: cf000000" hasStroke="1"
               stroke="2.7, mitered, butt" strokeColour="solid: ffa3a3a3"/>
    <ROUNDRECT pos="26 50 52M 132M" cornerSize="3" fill="solid: ffe9e9e9" hasStroke="1"
               stroke="2.7, mitered, butt" strokeColour="solid: ffa3a3a3"/>
  </BACKGROUND>
  <LABEL name="title label" id="d02617478241afef" memberName="titleLabel"
         virtualName="" explicitFocusOrder="0" pos="12 12 20M 24" textCol="ffc8c8c8"
         edTextCol="ff000000" edBkgCol="0" labelText="Status" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="17.199999999999999289" bold="1" italic="0" justification="33"/>
  <LABEL name="status message label" id="4372fb81c80e9436" memberName="statusMessageLabel"
         virtualName="" explicitFocusOrder="0" pos="32 48 64M 128M" edTextCol="ff000000"
         edBkgCol="0" labelText="Status message" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="36"/>
  <JUCERCOMP name="quit button" id="9b7116519798e593" memberName="quitButton"
             virtualName="" explicitFocusOrder="0" pos="216R 40R 98 24" sourceFile="DialogButtonComponent.cpp"
             constructorParams=""/>
  <JUCERCOMP name="connect button" id="79662722110a2713" memberName="connectButton"
             virtualName="" explicitFocusOrder="0" pos="112R 40R 98 24" sourceFile="DialogButtonComponent.cpp"
             constructorParams=""/>
  <TOGGLEBUTTON name="auto toggle button" id="47a03168a3b8035" memberName="autoToggleButton"
                virtualName="" explicitFocusOrder="0" pos="24 72R 48M 24" txtcol="ffeeeeee"
                buttonText="auto connect when single eigenD" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
