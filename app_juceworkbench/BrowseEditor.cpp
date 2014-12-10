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

#include "BrowseEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
BrowseEditor::BrowseEditor (Atom* atom, String name)
    : atom_(atom), name_(name)
{
    setName ("BrowseEditor");
    addAndMakeVisible (label = new Label ("new label",
                                          TRANS("label text")));
    label->setFont (Font (15.00f, Font::plain));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colours::white);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (comboBox = new ComboBox ("comboBox"));
    comboBox->setEditableText (false);
    comboBox->setJustificationType (Justification::centredLeft);
    comboBox->setTextWhenNothingSelected (String::empty);
    comboBox->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    comboBox->addListener (this);

    addAndMakeVisible (textEditor = new TextEditor ("textEditor"));
    textEditor->setMultiLine (false);
    textEditor->setReturnKeyStartsNewLine (false);
    textEditor->setReadOnly (true);
    textEditor->setScrollbarsShown (true);
    textEditor->setCaretVisible (false);
    textEditor->setPopupMenuEnabled (true);
    textEditor->setText (String::empty);

    addAndMakeVisible (browseButton = new TextButton ("browsebutton"));
    browseButton->setButtonText (TRANS("Browse"));
    browseButton->addListener (this);


    //[UserPreSize]
    treeContent_=0;
    textEditor->setVisible(false);
    browseButton->setVisible(false);
    comboBox->setColour(ComboBox::buttonColourId, Colour(0xffaeaeae));
    comboBox->setText("Fetching...",true);
    comboBox->setTooltip(String::empty);
    mode_=FLAT;
    //[/UserPreSize]

    setSize (400, 28);


    //[Constructor] You can add your own custom stuff here..
    label->setText(name,dontSendNotification);
    String tooltip= atom_->get_tooltip();
    if(tooltip.isNotEmpty())
    {
        label->setTooltip(name+ ": "+tooltip);
    }
    else
    {
        label->setTooltip(name);
    }


    atom_->enumerate("/root");
    //[/Constructor]
}

BrowseEditor::~BrowseEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    label = nullptr;
    comboBox = nullptr;
    textEditor = nullptr;
    browseButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    if( treeContent_!=0)
    {
        deleteAndZero (treeContent_);
    }
    //[/Destructor]
}

//==============================================================================
void BrowseEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void BrowseEditor::resized()
{
    label->setBounds (8, 4, 80, 20);
    comboBox->setBounds (getWidth() - 32 - 272, 8, 272, 16);
    textEditor->setBounds (96, 8, 224, 16);
    browseButton->setBounds (328, 8, 47, 16);
    //[UserResized] Add your own custom resize handling here..
    label->setBounds (getX(), 4, 72, 20);
    //[/UserResized]
}

void BrowseEditor::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == comboBox)
    {
        //[UserComboBoxCode_comboBox] -- add your combo box handling code here..
        String currentText=comboBox->getText();
        for(std::map<String, String>::const_iterator i=flatmap_.begin();i!=flatmap_.end();i++)
        {
           if(currentText==i->second)
           {
                atom_->activate(String::empty,i->first);
                break;
           }
        }
        comboBox->setTooltip(currentText);
        //[/UserComboBoxCode_comboBox]
    }

    //[UsercomboBoxChanged_Post]

    //[/UsercomboBoxChanged_Post]
}

void BrowseEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == browseButton)
    {
        //[UserButtonCode_browseButton] -- add your button handler code here..
        String title="Browsing " + atom_->get_fulldesc();
        DialogWindow::showModalDialog(title,treeContent_,this,Colour(0xffababab),true,true,true);

//        delete treeContent_;
//        treeContent_=0;
        getTopLevelComponent()->toFront(true);
        //[/UserButtonCode_browseButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void BrowseEditor::setChangingValue()
{
    if(mode_==TREE)
    {
        textEditor->setText("Setting value...",true);
        textEditor->setTooltip(String::empty);
    }
}

void BrowseEditor::activated()
{
    if(mode_==TREE)
    {
// XXX
//        textEditor->setText(atom_->get_value());
//        TreeViewItem* tvi=treeView_->getSelectedItem(0);
//        if (tvi!=0)
//        {
//            textEditor->setText(tvi->getUniqueName());
            atom_->get_current();
//        }
    }
}

void BrowseEditor::current(String cookie)
{
    if(mode_==TREE)
    {
        textEditor->setText(cookie);
        textEditor->setTooltip(cookie);
    }
    else
    {
        std::map<String,String>::iterator iter=flatmap_.find(cookie);
        if(iter!=flatmap_.end())
        {
            comboBox->setText(iter->second,true);
            comboBox->setTooltip(iter->second);
        }
        else
        {
            pic::logmsg()<<"current value "<<std::string(cookie.toUTF8())<<" not a key in finfo";
            comboBox->setText("Not set",true);
            comboBox->setTooltip(String::empty);
        }
    }
}

void BrowseEditor::enumerate_updated(String path,int nf,int nc)
{

    if(path=="/root"&& nc==0)
    {
        mode_=FLAT;
        if(nf==0)
        {
            // disable if nothing to show
            comboBox->setText("None available",true);
            comboBox->setEnabled(false);
        }
        else
        {
            atom_->finfo(String::empty);
        }
    }
    else
    {
        mode_=TREE;
        if(treeContent_==0)
        {
            comboBox->setText(String::empty,true);
            comboBox->setTooltip(String::empty);
            comboBox->setVisible(false);
            textEditor->setVisible(true);
            browseButton->setVisible(true);
            textEditor->setText(String::empty,false);
            textEditor->setTooltip (String::empty);

            pic::logmsg()<<"current value "<<std::string(atom_->get_value().toUTF8());
            // XXX
            // display cookie for the moment
//            textEditor->setText(atom_->get_value());
            atom_->get_current();

            // add a tree view
            treeContent_=new TreeDisplayComponent(atom_,nf,nc);
        }
        else
        {
            treeContent_->enumerate_updated(path, nf,nc);
        }
    }
}

void BrowseEditor::finfo_updated(String path, const std::set<std::string>& finfo)
{
    if(mode_==FLAT)
    {
        flat_populate(finfo);
    }
    else
    {
        treeContent_->finfo_updated(path,finfo);
    }
}

void BrowseEditor::cinfo_updated(String path,const std::set<std::string>& cinfo)
{
    if(mode_==FLAT)
    {
        // cant be flat with cinfo
    }
    else
    {
        treeContent_->cinfo_updated(path,cinfo);
    }
}

void BrowseEditor::flat_populate(const std::set<std::string>& finfo)
{
    int count=1;
    flatmap_.clear();
    std::set<std::string>::const_iterator i = finfo.begin();
    while(i!=finfo.end())
    {
        String s=String::fromUTF8(i->c_str());
        String k=s.upToFirstOccurrenceOf("&&",false,true);
        String v=s.fromLastOccurrenceOf("&&",false,true);
        flatmap_.insert(std::pair<String,String>(k,v));
        comboBox->addItem(v,count);
        count++;
        i++;
    }

    atom_->get_current();
}

void BrowseEditor::setIndent(int indent)
{
    label->setTopLeftPosition(indent,4);
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="BrowseEditor" componentName="BrowseEditor"
                 parentClasses="public Component" constructorParams="Atom* atom, String name"
                 variableInitialisers="atom_(atom), name_(name)" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="1"
                 initialWidth="400" initialHeight="28">
  <BACKGROUND backgroundColour="feffff"/>
  <LABEL name="new label" id="748a66cbe21e9af4" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="8 4 80 20" textCol="ffffffff" edTextCol="ff000000"
         edBkgCol="0" labelText="label text" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <COMBOBOX name="comboBox" id="4528118afaf322f0" memberName="comboBox" virtualName=""
            explicitFocusOrder="0" pos="32Rr 8 272 16" editable="0" layout="33"
            items="" textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <TEXTEDITOR name="textEditor" id="4bdd7d0288c5162" memberName="textEditor"
              virtualName="" explicitFocusOrder="0" pos="96 8 224 16" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="1" scrollbars="1"
              caret="0" popupmenu="1"/>
  <TEXTBUTTON name="browsebutton" id="948582498f31ebe4" memberName="browseButton"
              virtualName="" explicitFocusOrder="0" pos="328 8 47 16" buttonText="Browse"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
