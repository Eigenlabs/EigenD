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

#include "TabViewPreferencesComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
class ListMouseListener : public MouseListener
{
public:
    ListMouseListener(ListBox* const listbox, TickList* const ticklist) : listbox_(listbox), ticklist_(ticklist), dragstarted_(false), viewposition_(0) {}

    virtual void mouseDown(const MouseEvent& event)
    {
        dragstarted_ = false;
    }

    virtual void mouseDrag(const MouseEvent& event)
    {
        if(!dragstarted_ && event.getOffsetFromDragStart().y)
        {
            dragstarted_ = true;
            viewposition_ = listbox_->getViewport()->getViewPositionY();
        }
#if STAGE_BUILD==IOS
        if(dragstarted_)
        {
            listbox_->getViewport()->setViewPosition(0, viewposition_-event.getOffsetFromDragStart().y);
        }
#endif // STAGE_BUILD
    }

    virtual void mouseUp(const MouseEvent& event)
    {
        if (!dragstarted_ && listbox_->isEnabled())
        {
            MouseEvent e = event.getEventRelativeTo(listbox_);
            int row = listbox_->getRowContainingPosition(e.x, e.y);
            if(row >= 0)
            {
                ticklist_->tickListRowClicked(row, e);
            }
        }
        dragstarted_ = false;
    }

    ListBox* const listbox_;
    TickList* const ticklist_;
    bool dragstarted_;
    int viewposition_;
};
//[/MiscUserDefs]

//==============================================================================
TabViewPreferencesComponent::TabViewPreferencesComponent (StagePreferences* preferences)
    : preferences_(preferences)
{
    setName ("TabViewPreferences");
    addAndMakeVisible (aspectLabel = new Label ("aspectLabel",
                                                TRANS("Show Tabs by Aspect Ratio")));
    aspectLabel->setFont (Font (15.00f, Font::plain));
    aspectLabel->setJustificationType (Justification::centredLeft);
    aspectLabel->setEditable (false, false, false);
    aspectLabel->setColour (Label::textColourId, Colours::white);
    aspectLabel->setColour (TextEditor::textColourId, Colours::black);
    aspectLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (customLabel = new Label ("customLabel",
                                                TRANS("Custom Show Tabs")));
    customLabel->setFont (Font (15.00f, Font::plain));
    customLabel->setJustificationType (Justification::centredLeft);
    customLabel->setEditable (false, false, false);
    customLabel->setColour (Label::textColourId, Colours::white);
    customLabel->setColour (TextEditor::textColourId, Colours::black);
    customLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (tabList_ = new ListBox (TRANS("tabList"), &tabListModel_));
    tabList_->setName ("tabList");

    addAndMakeVisible (ratioList_ = new ListBox (TRANS("ratioList"), &ratioListModel_));
    ratioList_->setName ("ratioList");

    addAndMakeVisible (customToggleButton = new ToggleButton ("custom toggle button"));
    customToggleButton->setButtonText (TRANS("custom selection"));
    customToggleButton->addListener (this);
    customToggleButton->setColour (ToggleButton::textColourId, Colour (0xffcccccc));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (512, 512);


    //[Constructor] You can add your own custom stuff here..

    // set up list of tabs to show by aspect from preferences
    ratioListModel_.addItem("Monitor", preferences_->getBoolValue("monitor",true));
    ratioListModel_.addItem("Half Monitor Horizontal", preferences_->getBoolValue("halfMonitorHorizontal",true));
    ratioListModel_.addItem("Half Monitor Vertical", preferences_->getBoolValue("halfMonitorVertical",true));
    ratioListModel_.addItem("Widescreen Monitor", preferences_->getBoolValue("widescreenMonitor",true));
    ratioListModel_.addItem("Half Widescreen Monitor Horizontal", preferences_->getBoolValue("halfWidescreenMonitorHorizontal",true));
    ratioListModel_.addItem("Half Widescreen Monitor Vertical", preferences_->getBoolValue("halfWidescreenMonitorVertical",true));
    ratioListModel_.addItem("iPhone/iPod 3.5\" Horizontal", preferences_->getBoolValue("iPhoneiPodTouchHorizontal",false));
    ratioListModel_.addItem("iPhone/iPod 3.5\" Vertical", preferences_->getBoolValue("iPhoneiPodTouchVertical",false));
    ratioListModel_.addItem("iPhone/iPod 4\", 4.7\", 5.5\" Horizontal", preferences_->getBoolValue("iPhoneiPod4InchHorizontal",false));
    ratioListModel_.addItem("iPhone/iPod 4\", 4.7\", 5.5\" Vertical", preferences_->getBoolValue("iPhoneiPod4InchVertical",false));
    ratioListModel_.addItem("iPad Horizontal", preferences_->getBoolValue("iPadHorizontal",false));
    ratioListModel_.addItem("iPad Vertical", preferences_->getBoolValue("iPadVertical",false));
    ratioListModel_.addItem("Custom", preferences_->getBoolValue("custom",true));
    ratioList_->updateContent();
    ratioListModel_.setListBox(ratioList_);
    ratioListModel_.setTabListModel(&tabListModel_);
    ratioListModel_.setCustomToggleButton(customToggleButton);

    customToggleButton->setWantsKeyboardFocus(false);
    customToggleButton->setToggleState(preferences_->getBoolValue("customShowTabs", false), dontSendNotification);

    tabList_->setEnabled(customToggleButton->getToggleState());
    for (int i=0; i<tabListModel_.getNumRows(); i++)
    {
        tabList_->repaintRow(i);
    }
#if STAGE_BUILD==IOS
    ratioList_->getViewport()->setScrollBarsShown(false, false);
    tabList_->getViewport()->setScrollBarsShown(false, false);
#endif // STAGE_BUILD
    ratioListListener_ = new ListMouseListener(ratioList_, &ratioListModel_);
    ratioList_->addMouseListener(ratioListListener_, true);
    tabListListener_ = new ListMouseListener(tabList_, &tabListModel_);
    tabList_->addMouseListener(tabListListener_, true);

    //[/Constructor]
}

TabViewPreferencesComponent::~TabViewPreferencesComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    aspectLabel = nullptr;
    customLabel = nullptr;
    tabList_ = nullptr;
    ratioList_ = nullptr;
    customToggleButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    tabListListener_=nullptr;
    ratioListListener_=nullptr;
    //[/Destructor]
}

//==============================================================================
void TabViewPreferencesComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
// black background just used in jucer
#if 0
    //[/UserPrePaint]

    g.fillAll (Colours::black);

    //[UserPaint] Add your own custom painting code here..
#endif
    g.setColour (Colour (0xffe9e9e9));
    g.fillRoundedRectangle ((float)ratioList_->getX() - 2, (float)ratioList_->getY() - 2, (float)ratioList_->getWidth() + 4, (float)ratioList_->getHeight() + 4, 3.0000f);

    g.setColour (Colour (0xffa3a3a3));
    g.drawRoundedRectangle ((float)ratioList_->getX() - 2, (float)ratioList_->getY() - 2, (float)ratioList_->getWidth() + 4, (float)ratioList_->getHeight() + 4, 3.0000f, 2.7000f);

    g.setColour (Colour (0xffe9e9e9));
    g.fillRoundedRectangle ((float)tabList_->getX() - 2, (float)tabList_->getY() - 2, (float)tabList_->getWidth() + 4, (float)tabList_->getHeight() + 4, 3.0000f);

    g.setColour (Colour (0xffa3a3a3));
    g.drawRoundedRectangle ((float)tabList_->getX() - 2, (float)tabList_->getY() - 2, (float)tabList_->getWidth() + 4, (float)tabList_->getHeight() + 4, 3.0000f, 2.7000f);

    g.fillAll (Colour(0x0));
    //[/UserPaint]
}

void TabViewPreferencesComponent::resized()
{
    aspectLabel->setBounds (0, 0, 256, 24);
    customLabel->setBounds (0, ((0) + (24) - -8) + (proportionOfHeight (0.4004f)) - -6, 224, 24);
    tabList_->setBounds (8, (((0) + (24) - -8) + (proportionOfHeight (0.4004f)) - -6) + (24) - -8, getWidth() - 16, (proportionOfHeight (0.4004f)) - 0);
    ratioList_->setBounds (8, (0) + (24) - -8, getWidth() - 16, proportionOfHeight (0.4004f));
    customToggleButton->setBounds (135, (((0) + (24) - -8) + (proportionOfHeight (0.4004f)) - -6) + 0, 160, 24);
    //[UserResized] Add your own custom resize handling here..
	int tabListY = customToggleButton->getY() + customToggleButton->getHeight() + 7;
    tabList_->setBounds (8, tabListY, getWidth() - 16, (proportionOfHeight (1.f)) - tabListY - 8);
    //[/UserResized]
}

void TabViewPreferencesComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == customToggleButton)
    {
        //[UserButtonCode_customToggleButton] -- add your button handler code here..
        tabList_->setEnabled(customToggleButton->getToggleState());
        for (int i=0; i<tabListModel_.getNumRows(); i++)
            tabList_->repaintRow(i);
        //[/UserButtonCode_customToggleButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void TabViewPreferencesComponent::setTabbedComponent (WidgetTabbedComponent* tabbedComponent)
{
    tabbedComponent_ = tabbedComponent;

    // get tab names and aspect modes to build the custom tab list
    int numTabs = tabbedComponent_->getNumTabs();

    for (int i=0; i<numTabs; i++)
    {
        WidgetViewComponent* widgetView = dynamic_cast<WidgetViewComponent*> (tabbedComponent_->getTabContentComponent(i));
        String name = widgetView->getTabName();
        int aspectMode;
        float aspectWidth, aspectHeight;
        widgetView->getAspect(aspectMode, aspectWidth, aspectHeight);

        tabListModel_.addItem(name, aspectMode, tabbedComponent_->getTabVisibleXml(i));
    }

    tabList_->updateContent();
    tabListModel_.setListBox(tabList_);

}


void TabViewPreferencesComponent::applyChanges()
{
    // apply list of tabs to show by aspect to preferences

    preferences_->setValue("monitor", ratioListModel_.getItemTicked(0));
    preferences_->setValue("halfMonitorHorizontal", ratioListModel_.getItemTicked(1));
    preferences_->setValue("halfMonitorVertical", ratioListModel_.getItemTicked(2));
    preferences_->setValue("widescreenMonitor", ratioListModel_.getItemTicked(3));
    preferences_->setValue("halfWidescreenMonitorHorizontal", ratioListModel_.getItemTicked(4));
    preferences_->setValue("halfWidescreenMonitorVertical", ratioListModel_.getItemTicked(5));
    preferences_->setValue("iPhoneiPodTouchHorizontal", ratioListModel_.getItemTicked(6));
    preferences_->setValue("iPhoneiPodTouchVertical", ratioListModel_.getItemTicked(7));
    preferences_->setValue("iPhoneiPod4InchHorizontal", ratioListModel_.getItemTicked(8));
    preferences_->setValue("iPhoneiPod4InchVertical", ratioListModel_.getItemTicked(9));
    preferences_->setValue("iPadHorizontal", ratioListModel_.getItemTicked(10));
    preferences_->setValue("iPadVertical", ratioListModel_.getItemTicked(11));
    preferences_->setValue("custom", ratioListModel_.getItemTicked(12));

    // apply custom tabs to show to tabs
    bool customShowTabs = customToggleButton->getToggleState();
    preferences_->setValue("customShowTabs", customShowTabs);

    if(customShowTabs)
    {
        // set tabs according to custom list
        for (int i=0; i<tabbedComponent_->getNumTabs(); i++)
            tabbedComponent_->setTabVisible(i, tabListModel_.getItemTicked(i));
    }
    else
    {
        // set tabs according to aspect preferences
        for (int i=0; i<tabbedComponent_->getNumTabs(); i++)
            tabbedComponent_->setTabVisible(i, ratioListModel_.getItemTicked(tabListModel_.getAspect(i)-1));
    }

    if (tabListModel_.getChanged())
    {
        // set custom list in xml if it has changed
        for (int i=0; i<tabbedComponent_->getNumTabs(); i++)
            tabbedComponent_->setTabVisibleXml(i, tabListModel_.getItemTicked(i));
    }

    // bring a visible tab into view
    tabbedComponent_->ensureCurrentTabVisible();
}


//==============================================================================
// TickList
//
// ListBoxModel with tickable items
//==============================================================================

void TickList::addItem(String name, bool ticked)
{
    items_.add(name);
    ticked_.add(ticked);
}


int TickList::getNumRows()
{
    return items_.size();
}

void TickList::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
    if(listBox_)
    {
        if (listBox_->isEnabled())
            g.setColour (Colours::black);
        else
            g.setColour (Colours::grey);
    }

    g.setFont (height * 0.7f);
    Font font = g.getCurrentFont();

    const float halfH = height * 0.5f;
    const int leftBorder = (height * 5) / 4;
    const int rightBorder = 4;

    if (ticked_[rowNumber])
    {
        const Path tick (LookAndFeel::getDefaultLookAndFeel().getTickShape (1.0f));
        const float th = font.getAscent();
        const float ty = halfH - th * 0.5f;

        g.fillPath (tick, tick.getTransformToScaleToFit (2.0f, ty, (float) (leftBorder - 4), th, true));
    }

    g.drawFittedText (items_[rowNumber], leftBorder, 0, width - (leftBorder + rightBorder), height,
                      Justification::centredLeft, 1, 1.0f);
}

void TickList::tickListRowClicked (int row, const MouseEvent &e)
{
}

void TickList::returnKeyPressed (int lastRowSelected)
{
}

void RatioTickList::tickListRowClicked (int row, const MouseEvent &e)
{
    // apply this aspect to
    bool ticked = !ticked_[row];
    ticked_.set(row, ticked);
    if(listBox_)
        listBox_->repaintRow(row);

    if(customToggleButton_->getToggleState())
    {
        tabListModel_->changed_ = true;
        for (int i=0; i<tabListModel_->aspectMode_.size(); i++)
        {
            if (tabListModel_->aspectMode_[i]==row+1)
            {
                tabListModel_->ticked_.set(i, ticked_[row]);
                tabListModel_->listBox_->repaintRow(i);
            }
        }
    }
}

void TabTickList::addItem(String name, int aspectMode, bool ticked)
{
    items_.add(name);
    aspectMode_.add(aspectMode);
    ticked_.add(ticked);
}

void TabTickList::tickListRowClicked (int row, const MouseEvent &e)
{
    changed_ = true;
    bool ticked = !ticked_[row];
    ticked_.set(row, ticked);
    if(listBox_)
    {
        listBox_->repaintRow(row);
    }
}


//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="TabViewPreferencesComponent"
                 componentName="TabViewPreferences" parentClasses="public Component"
                 constructorParams="StagePreferences* preferences" variableInitialisers="preferences_(preferences)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="512" initialHeight="512">
  <BACKGROUND backgroundColour="ff000000"/>
  <LABEL name="aspectLabel" id="eaef2fdce95e374c" memberName="aspectLabel"
         virtualName="" explicitFocusOrder="0" pos="0 0 256 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Show Tabs by Aspect Ratio"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="customLabel" id="26e7bc7cc27b72cf" memberName="customLabel"
         virtualName="" explicitFocusOrder="0" pos="0 -6R 224 24" posRelativeY="e0c793d88645b47"
         textCol="ffffffff" edTextCol="ff000000" edBkgCol="0" labelText="Custom Show Tabs"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <GENERICCOMPONENT name="tabList" id="c2d653ed3e887aac" memberName="tabList_" virtualName=""
                    explicitFocusOrder="0" pos="8 -8R 16M 0M" posRelativeY="26e7bc7cc27b72cf"
                    posRelativeH="e0c793d88645b47" class="ListBox" params="T(&quot;tabList&quot;), &amp;tabListModel_"/>
  <GENERICCOMPONENT name="ratioList" id="e0c793d88645b47" memberName="ratioList_"
                    virtualName="" explicitFocusOrder="0" pos="8 -8R 16M 40.039%"
                    posRelativeY="eaef2fdce95e374c" class="ListBox" params="T(&quot;ratioList&quot;), &amp;ratioListModel_"/>
  <TOGGLEBUTTON name="custom toggle button" id="a5577ae29625e5b1" memberName="customToggleButton"
                virtualName="" explicitFocusOrder="0" pos="135 0 160 24" posRelativeY="26e7bc7cc27b72cf"
                txtcol="ffcccccc" buttonText="custom selection" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
