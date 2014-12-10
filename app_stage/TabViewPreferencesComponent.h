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

#ifndef __JUCE_HEADER_C93B1B8781661DBA__
#define __JUCE_HEADER_C93B1B8781661DBA__

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

#include "WidgetTabbedComponent.h"

class TickList : public ListBoxModel
{
public:
    TickList() : listBox_(0) {}
    ~TickList() {}

    void addItem(String name, bool ticked=false);

    void setListBox(ListBox* listBox) { listBox_ = listBox; }
    void setItemTicked(int index, bool ticked) { ticked_.set(index, ticked); }
    bool getItemTicked(int index) { return ticked_[index]; }

    int getNumRows();
    void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected);

    virtual void tickListRowClicked(int row, const MouseEvent &e);
    void returnKeyPressed (int lastRowSelected);

protected:

    Array<String> items_;
    Array<bool> ticked_;

    ListBox* listBox_;
};

// tick list model for custom tab list
class TabTickList : public TickList
{
public:
    TabTickList() : changed_(false) {}
    ~TabTickList() {}

    void addItem(String name, int aspect, bool ticked=false);
    bool getChanged() { return changed_; }
    int getAspect(int index) { return aspectMode_[index]; }

    virtual void tickListRowClicked(int row, const MouseEvent &e);

    friend class RatioTickList;

private:
    Array<int> aspectMode_;
    bool changed_;
};

// tick list model for aspect ratio tab list
class RatioTickList : public TickList
{
public:
    RatioTickList() {}
    ~RatioTickList() {}

    void setTabListModel(TabTickList* tabListModel) { tabListModel_ = tabListModel; }
    void setCustomToggleButton(ToggleButton* customToggleButton) { customToggleButton_ = customToggleButton; }

    virtual void tickListRowClicked(int row, const MouseEvent &e);

private:
    TabTickList* tabListModel_;
    ToggleButton* customToggleButton_;
};


//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class TabViewPreferencesComponent  : public Component,
                                     public ButtonListener
{
public:
    //==============================================================================
    TabViewPreferencesComponent (StagePreferences* preferences);
    ~TabViewPreferencesComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void setTabbedComponent (WidgetTabbedComponent* tabbedComponent);
    void applyChanges();
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    StagePreferences* preferences_;

    TabTickList tabListModel_;
    RatioTickList ratioListModel_;
    ScopedPointer<MouseListener> tabListListener_;
    ScopedPointer<MouseListener> ratioListListener_;


    WidgetTabbedComponent* tabbedComponent_;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Label> aspectLabel;
    ScopedPointer<Label> customLabel;
    ScopedPointer<ListBox> tabList_;
    ScopedPointer<ListBox> ratioList_;
    ScopedPointer<ToggleButton> customToggleButton;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TabViewPreferencesComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_C93B1B8781661DBA__
