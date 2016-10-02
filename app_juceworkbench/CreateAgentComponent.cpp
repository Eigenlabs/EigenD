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

#include "CreateAgentComponent.h"

class AgentListBox : public ListBox 
{
public:
    AgentListBox ( const String& componentName, ListBoxModel* model, StringArray& agentNames) 
    : ListBox(componentName,model),agentNames_(agentNames)
    {
    }

    bool keyPressed (const KeyPress& key) override 
    {
        juce_wchar kc = key.getTextCharacter();
        if(CharacterFunctions::isLetter(kc)) 
        {
            juce_wchar lkc = CharacterFunctions::toLowerCase(kc);
            juce_wchar ukc = CharacterFunctions::toUpperCase(kc);
            for(int i=0;i<agentNames_.size();i++) 
            {
                if(agentNames_[i].startsWithChar(lkc) || agentNames_[i].startsWithChar(ukc)) 
                {
                    selectRow(i);
                    return true;
                }
            }
            return true;
        }
        return ListBox::keyPressed(key);
    }

 private:
     StringArray& agentNames_;
};

CreateAgentComponent::CreateAgentComponent (const std::set<std::string>& agents, Workspace* workspace)
    : agents_(agents),
      ordinal_(1),
      listBox(0),
      textButton (0),
      textButton2 (0),
      txtOrdinal (0),
      upButton (0),
      downButton (0),
      label (0),
      errorLabel (0),
      workspace_(workspace),
      availableLabel(0),
      createLabel(0),
      selectedAgentLabel(0)
{
    addAndMakeVisible(listBox = new AgentListBox("agentBox",this,agentNames_));
    addAndMakeVisible (textButton = new TextButton ("okButton"));
    textButton->setButtonText ("OK");
    textButton->addListener (this);
    textButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (textButton2 = new TextButton ("cancelButton"));
    textButton2->setButtonText ("Cancel");
    textButton2->addListener (this);
    textButton2->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (txtOrdinal = new TextEditor ("txtOrdinal"));
    txtOrdinal->setMultiLine (false);
    txtOrdinal->setReturnKeyStartsNewLine (false);
    txtOrdinal->setReadOnly (false);
    txtOrdinal->setScrollbarsShown (false);
    txtOrdinal->setCaretVisible (true);
    txtOrdinal->setPopupMenuEnabled (false);
    txtOrdinal->setText ("1");

    addAndMakeVisible (upButton = new TextButton ("upButton"));
    upButton->setTooltip ("Increment No.");
    upButton->setButtonText ("+");
    upButton->addListener (this);
    upButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (downButton = new TextButton ("downButton"));
    downButton->setTooltip ("Decrement No.");
    downButton->setButtonText ("-");
    downButton->addListener (this);
    downButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (label = new Label ("new label",
                                          "No."));
    label->setFont (Font (15.0000f, Font::plain));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colours::white);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (availableLabel = new Label ("availablelabel",
                                          "Available agents:"));
    availableLabel->setFont (Font (15.0000f, Font::plain));
    availableLabel->setJustificationType (Justification::centredLeft);
    availableLabel->setEditable (false, false, false);
    availableLabel->setColour (Label::textColourId, Colours::white);
    availableLabel->setColour (TextEditor::textColourId, Colours::black);
    availableLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (createLabel = new Label ("createlabel",
                                          "Create:"));
    createLabel->setFont (Font (15.0000f, Font::plain));
    createLabel->setJustificationType (Justification::centredLeft);
    createLabel->setEditable (false, false, false);
    createLabel->setColour (Label::textColourId, Colours::white);
    createLabel->setColour (TextEditor::textColourId, Colours::black);
    createLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (selectedAgentLabel = new Label ("selectedAgentLabel",
                                          ""));
    selectedAgentLabel->setFont (Font (15.0000f, Font::plain));
    selectedAgentLabel->setJustificationType (Justification::centredLeft);
    selectedAgentLabel->setEditable (false, false, false);
    selectedAgentLabel->setColour (Label::textColourId, Colours::white);
    selectedAgentLabel->setColour (TextEditor::textColourId, Colours::black);
    selectedAgentLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (detailsLabel = new TextEditor("detailsLabel"));
    detailsLabel->setMultiLine(true);
//    detailsLabel->setFont (Font (15.0000f, Font::plain));
//    detailsLabel->setJustificationType (Justification::centredLeft);
//    detailsLabel->setEditable (false, false, false);
    detailsLabel->setReadOnly(true);
//    detailsLabel->setColour (Label::textColourId, Colours::white);
//    detailsLabel->setColour (TextEditor::textColourId, Colours::black);
//    detailsLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (errorLabel = new Label ("errorLabel",
                                               "Number already in use!"));
    errorLabel->setFont (Font (15.0000f, Font::plain));
    errorLabel->setJustificationType (Justification::centredLeft);
    errorLabel->setEditable (false, false, false);
    errorLabel->setColour (Label::textColourId, Colour (0xffe76a3d));
    errorLabel->setColour (TextEditor::textColourId, Colours::black);
    errorLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));


    okPressed_=false;
    setupOrdinals();

    std::set<std::string>::const_iterator i = agents_.begin();
    agentNames_=StringArray();

    while(i!=agents_.end())
    {
        String s = String::fromUTF8(i->c_str());
        agentNames_.add(s.upToFirstOccurrenceOf(",",false,true));
        i++;
    }

    listBox->updateContent();
    listBox->selectRow(0,false,true);
    //listBox->setMouseMoveSelectsRows(true);

    errorLabel->setVisible(false);
    txtOrdinal->addListener(this);
    setLowestAvailableOrdinal();
    txtOrdinal->setText (String(ordinal_));

    setWantsKeyboardFocus(true);
    grabKeyboardFocus();
    toFront(true);
    setSize (300, 400);
    listBox->toFront(true);
    listBox->setWantsKeyboardFocus(true);
    listBox->grabKeyboardFocus();
}

CreateAgentComponent::~CreateAgentComponent()
{
    deleteAndZero (listBox);
    deleteAndZero (textButton);
    deleteAndZero (textButton2);
    deleteAndZero (txtOrdinal);
    deleteAndZero (upButton);
    deleteAndZero (downButton);
    deleteAndZero (label);
    deleteAndZero (errorLabel);
    deleteAndZero (availableLabel);
    deleteAndZero (createLabel);
    deleteAndZero (selectedAgentLabel);
    deleteAndZero (detailsLabel);
}

void CreateAgentComponent::paint (Graphics& g)
{

    g.setGradientFill (ColourGradient (Colour (0xff060505),
                                       96.0f, 0.0f,
                                       Colour (0xff757775),
                                       100.0f, 400.0f,
                                       false));
    g.fillRect (0, 0, proportionOfWidth (1.0000f), proportionOfHeight (1.0000f));

}

void CreateAgentComponent::resized()
{
    availableLabel->setBounds(20,18,200,24);
    listBox->setBounds(24,40,248,150);

    createLabel->setBounds(20,205,200,24);
    selectedAgentLabel->setBounds(20,225,150,24);
    label->setBounds (152, 225, 28, 24);
    txtOrdinal->setBounds (180, 225, 56, 24);
    upButton->setBounds (240, 225, 32, 11);
    downButton->setBounds (240, 238, 32, 11);

    errorLabel->setBounds (22, 250, 232, 16);

    detailsLabel->setBounds(24,260,248,90);
    textButton->setBounds (200, 360, 71, 24);
    textButton2->setBounds (112, 360, 71, 24);
}

int CreateAgentComponent::getNumRows()
{
    return agents_.size();
}

void CreateAgentComponent::paintListBoxItem(int rowNumber,Graphics& g, int width, int height, bool rowIsSelected)
{
   if(rowIsSelected)
   {
       g.fillAll(Colour(0xff868686));
       g.fillAll(Colour(0xffb8b8b8));
   }
   g.drawSingleLineText(agentNames_[rowNumber], 2,height-6);
}

void CreateAgentComponent::selectedRowsChanged(int lastRowSelected)
{
    pic::logmsg()<<"selectedRowsChangedi last RowSelected"<<lastRowSelected;
    setLowestAvailableOrdinal();
    txtOrdinal->setText (String(ordinal_));
    String s=agentNames_[listBox->getSelectedRow()];
    selectedAgentLabel->setText(s,dontSendNotification);
    detailsLabel->setText(workspace_->get_helptext(s),dontSendNotification);
}

void CreateAgentComponent::listBoxItemDoubleClicked(int row, const MouseEvent& e)
{
    pic::logmsg()<<"listBoxItemDoubleClicked row="<<row;
    DialogWindow* dw =findParentComponentOfClass<DialogWindow>();
    if(dw!=0)
    {
        okPressed_=true;
        dw->exitModalState(0);
    }

}

void CreateAgentComponent::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == textButton)
    {
        DialogWindow* dw =findParentComponentOfClass<DialogWindow>();
        if(dw!=0)
        {
            okPressed_=true;
            dw->exitModalState(0);
        }
    }
    else if (buttonThatWasClicked == textButton2)
    {
        DialogWindow* dw =findParentComponentOfClass<DialogWindow>();
        if(dw!=0)
        {
            dw->exitModalState(0);
        }
    }
    else if (buttonThatWasClicked == upButton)
    {
        setNextAvailableOrdinal();
        txtOrdinal->setText (String(ordinal_));
    }
    else if (buttonThatWasClicked == downButton)
    {
        setPreviousAvailableOrdinal();
        txtOrdinal->setText (String(ordinal_));
    }

}

void CreateAgentComponent::setupOrdinals()
{
    std::set<std::string>::const_iterator i = agents_.begin();

    String testKey;
    String ordinals;
    String o;

    while(i!=agents_.end())
    {
        String s = String::fromUTF8(i->c_str());

        testKey=(s.upToFirstOccurrenceOf(",",false,true));
        ordinals=s.fromFirstOccurrenceOf(",",false,true);

        std::set<int> testOrdinals;
        while(ordinals.isNotEmpty())
        {
            o=ordinals.upToFirstOccurrenceOf(",",false,true);
            ordinals=ordinals.fromFirstOccurrenceOf(",",false,true);
            testOrdinals.insert(o.getIntValue());
        }

        ordinalMap_.insert(std::pair<String,std::set<int> >(testKey,testOrdinals));
        i++;
    }
}

void CreateAgentComponent::setSelection(String selection)
{

    std::set<std::string>::const_iterator i = agents_.begin();
    int count=0;
    while(i!=agents_.end())
    {
        String s = String::fromUTF8(i->c_str());
        if(s.upToFirstOccurrenceOf(",",false,true)==selection)
        {
            listBox->selectRow(count,false,true);
            setLowestAvailableOrdinal();
            txtOrdinal->setText (String(ordinal_));
            break;
        }
        i++;
        count++;
    }

}

String CreateAgentComponent::getTooltipForRow(int row)
{
    return workspace_->get_tooltip(agentNames_[row]);
}

String CreateAgentComponent::getSelection()
{
      return agentNames_[listBox->getSelectedRow()];
}

int CreateAgentComponent::getOrdinal()
{
    return ordinal_;
}

void CreateAgentComponent::setNextAvailableOrdinal()
{

    std::map<String,std::set<int> >::iterator i=ordinalMap_.find(getSelection());
    if(i!=ordinalMap_.end())
    {
        int test=ordinal_+1;
        std::set<int> in_use= (i->second);
        std::set<int>::iterator iter;
        iter=in_use.find(test);

        while(iter!=in_use.end())
        {
            test=test+1;
            iter=in_use.find(test);
        }

        ordinal_=test;
    }
    else
    {
        pic::logmsg()<<"cant find key in ordinalMap"<<std::string(getSelection().toUTF8());
    }
}

void CreateAgentComponent::setPreviousAvailableOrdinal()
{
    if (ordinal_>1)
    {
        std::map<String,std::set<int> >::iterator i=ordinalMap_.find(getSelection());
        if(i!=ordinalMap_.end())
        {
            int test=ordinal_-1;
            std::set<int> in_use= (i->second);
            std::set<int>::iterator iter;
            iter=in_use.find(test);

            while(iter!=in_use.end() && (test!=0))
            {
                test=test-1;
                iter=in_use.find(test);
            }

            if (test>0)
            {
                ordinal_=test;
            }
        }
        else
        {
            pic::logmsg()<<"cant find key in ordinalMap"<<std::string(getSelection().toUTF8());
        }

    }
}

void CreateAgentComponent::setLowestAvailableOrdinal()
{
    std::map<String,std::set<int> >::iterator i=ordinalMap_.find(getSelection());
    if(i!=ordinalMap_.end())
    {
        int test=1;
        std::set<int> in_use= (i->second);
        std::set<int>::iterator iter;
        iter=in_use.find(test);

        while(iter!=in_use.end())
        {
            test=test+1;
            iter=in_use.find(test);
        }

        ordinal_=test;
    }
    else
    {
        pic::logmsg()<<"cant find key in ordinalMap"<<std::string(getSelection().toUTF8());
    }
}

bool CreateAgentComponent::isValidOrdinal(String ord)
{
    int test=ord.getIntValue();
    std::map<String,std::set<int> >::iterator i=ordinalMap_.find(getSelection());
    if(i!=ordinalMap_.end())
    {
        std::set<int> in_use= (i->second);
        std::set<int>::iterator iter;
        iter=in_use.find(test);
        if(iter!=in_use.end())
        {
            return false;
        }
    }
    return true;
}

void CreateAgentComponent:: textEditorTextChanged(TextEditor &editor)
{
    if(&editor==txtOrdinal)
    {
        if(!isValidOrdinal(txtOrdinal->getText()))
        {
            textButton->setEnabled(false);
            errorLabel->setVisible(true);
        }
        else
        {
            ordinal_=txtOrdinal->getText().getIntValue();
            textButton->setEnabled(true);
            errorLabel->setVisible(false);
        }

    }
}
