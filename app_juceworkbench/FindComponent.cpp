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
 Copyright 2012-2014 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#include "FindComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
FindComponent::FindComponent (std::vector<String> names)
{
    addAndMakeVisible (findButton = new TextButton ("findButton"));
    findButton->setButtonText (TRANS("Find"));
    findButton->addListener (this);
    findButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (cancelButton = new TextButton ("cancelButton"));
    cancelButton->setButtonText (TRANS("Cancel"));
    cancelButton->addListener (this);
    cancelButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (listbox = new ListBox());
    listbox->setName ("listbox");


    //[UserPreSize]
    listbox->setModel(this);
    initialised_=false;
    names_=names;
    okPressed_=false;
    //[/UserPreSize]

    setSize (300, 350);


    //[Constructor] You can add your own custom stuff here..

    for (std::vector<String>::iterator i=names.begin();i!=names.end();i++)
    {
        displayNames_.add(*i);
    }

    listbox->updateContent();
    listbox->selectRow(0,false,true);


    //[/Constructor]
}

FindComponent::~FindComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    findButton = nullptr;
    cancelButton = nullptr;
    listbox = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void FindComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    g.setGradientFill (ColourGradient (Colour (0xff161616),
                                       96.0f, 0.0f,
                                       Colour (0xff898989),
                                       104.0f, 400.0f,
                                       false));
    g.fillRect (-8, -8, 312, 408);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void FindComponent::resized()
{
    findButton->setBounds (56, 312, 80, 24);
    cancelButton->setBounds (176, 312, 70, 24);
    listbox->setBounds (24, 24, 256, 272);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void FindComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == findButton)
    {
        //[UserButtonCode_findButton] -- add your button handler code here..
        DialogWindow* dw =findParentComponentOfClass<DialogWindow>();
        if(dw!=0)
        {
            okPressed_=true;
            dw->exitModalState(0);
        }

        //[/UserButtonCode_findButton]
    }
    else if (buttonThatWasClicked == cancelButton)
    {
        //[UserButtonCode_cancelButton] -- add your button handler code here..
        DialogWindow* dw =findParentComponentOfClass<DialogWindow>();
        if(dw!=0)
        {
            dw->exitModalState(0);
        }

        //[/UserButtonCode_cancelButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

bool FindComponent::keyPressed (const KeyPress& key)
{
    //[UserCode_keyPressed] -- Add your code here...
    if(key.getKeyCode()==KeyPress::returnKey)
    {
        std::cout<<"return key pressed"<<std::endl;
        return true;
    }
    return false;  // Return true if your handler uses this key event, or false to allow it to be passed-on.
    //[/UserCode_keyPressed]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

String FindComponent::getSelection()
{
      return displayNames_[listbox->getSelectedRow()];
}


void FindComponent::setFocusOrder()
{
    //txtFind->setWantsKeyboardFocus(true);
    //txtFind->setExplicitFocusOrder(1);
}

//void::FindComponent::textEditorReturnKeyPressed(TextEditor &editor)
//{
//    DialogWindow* dw =findParentComponentOfClass<DialogWindow>();
//    if(dw!=0)
//    {
//        okPressed_=true;
//        dw->exitModalState(0);
//    }
//}

int FindComponent::getNumRows()
{
    return names_.size();
}

void FindComponent::paintListBoxItem(int rowNumber,Graphics& g, int width, int height, bool rowIsSelected)
{
   if(rowIsSelected)
   {
       g.fillAll(Colour(0xff868686));
       g.fillAll(Colour(0xffb8b8b8));
   }
   g.drawSingleLineText(displayNames_[rowNumber], 2,height-6);
}

void FindComponent::selectedRowsChanged(int lastRowSelected)
{
    pic::logmsg()<<"selectedRowsChangedi last RowSelected"<<lastRowSelected;
}

void FindComponent::listBoxItemDoubleClicked(int row, const MouseEvent& e)
{
    pic::logmsg()<<"listBoxItemDoubleClicked row="<<row;
    DialogWindow* dw =findParentComponentOfClass<DialogWindow>();
    if(dw!=0)
    {
        okPressed_=true;
        dw->exitModalState(0);
    }
}

String FindComponent::getTooltipForRow(int row)
{
    return String::empty;
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="FindComponent" componentName=""
                 parentClasses="public Component, public ListBoxModel" constructorParams="std::vector&lt;String&gt; names"
                 variableInitialisers="" snapPixels="8" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="1" initialWidth="300" initialHeight="350">
  <METHODS>
    <METHOD name="keyPressed (const KeyPress&amp; key)"/>
  </METHODS>
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="-8 -8 312 408" fill="linear: 96 0, 104 400, 0=ff161616, 1=ff898989"
          hasStroke="0"/>
  </BACKGROUND>
  <TEXTBUTTON name="findButton" id="db06151ac7f364db" memberName="findButton"
              virtualName="" explicitFocusOrder="0" pos="56 312 80 24" bgColOff="ffaeaeae"
              buttonText="Find" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="cancelButton" id="64a84dc445c9601" memberName="cancelButton"
              virtualName="" explicitFocusOrder="0" pos="176 312 70 24" bgColOff="ffaeaeae"
              buttonText="Cancel" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <GENERICCOMPONENT name="listbox" id="bd01d9506f77825d" memberName="listbox" virtualName="ListBox"
                    explicitFocusOrder="0" pos="24 24 256 272" class="Component"
                    params=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
