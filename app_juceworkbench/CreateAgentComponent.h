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

#ifndef __CREATEAGENTCOMP__
#define __CREATEAGENTCOMP__
#include <piw/piw_tsd.h>
#include "juce.h"
#include "Workspace.h"
#include <set>
#include <map>

class CreateAgentComponent  : public Component,
                             public TextEditor::Listener,
                             public ButtonListener,
                             public ListBoxModel
{
public:
    CreateAgentComponent (const std::set<std::string>& agents, Workspace* workspace);
    ~CreateAgentComponent();

    String getSelection();
    void setSelection(String selection);
    int getOrdinal();
    bool okPressed_;
    virtual void textEditorTextChanged(TextEditor &editor);
    virtual void textEditorReturnKeyPressed(TextEditor &editor){};
    virtual void textEditorEscapeKeyPressed(TextEditor &editor){};
    virtual void textEditorFocusLost(TextEditor &editor){};

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);
    virtual int getNumRows();
    virtual void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected);
    virtual void selectedRowsChanged(int lastRowSelected);
    virtual String getTooltipForRow(int row);
    virtual void listBoxItemDoubleClicked(int row, const MouseEvent& e);

private:
    std::set<std::string> agents_;
    StringArray agentNames_;
    void setNextAvailableOrdinal();
    void setPreviousAvailableOrdinal();
    void setLowestAvailableOrdinal();
    void setupOrdinals();
    std::map<String,std::set<int> > ordinalMap_;
    bool isValidOrdinal(String ord);
    int ordinal_;

    ListBox* listBox;
    TextButton* textButton;
    TextButton* textButton2;
    TextEditor* txtOrdinal;
    TextButton* upButton;
    TextButton* downButton;
    Label* label;
    Label* errorLabel;
    Workspace* workspace_;
    Label* availableLabel;
    Label* createLabel;
    Label* selectedAgentLabel;
    TextEditor* detailsLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CreateAgentComponent);
};


#endif
