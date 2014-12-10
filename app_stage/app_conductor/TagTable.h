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

#ifndef __TAG_TABLE__
#define __TAG_TABLE__

#include "juce.h"
#include "Conductor.h"
#include <set>
#include <map>
#include <vector>
#include "Clip.h"
#include "ClipManagerPanel.h"

class CellEditor:public TextEditor
{
public:    
    CellEditor(String category,String oldText);
    ~CellEditor(){};
    String getCategory();
    String getOldText();
    virtual void focusGained(FocusChangeType);
    virtual void focusLost(FocusChangeType);
private:
    String category_;
    String oldText_;
};

class TagEditTableHeader: public TableHeaderComponent
{
public:
    TagEditTableHeader(ClipManagerBackend*);
    ~TagEditTableHeader(){};
    virtual void columnClicked(int columnId, const ModifierKeys &mods);

private:
    ClipManagerBackend* backend_;
};


class TagTableHeader: public TableHeaderComponent
{
public:
    TagTableHeader(ClipManagerBackend*);
    ~TagTableHeader(){};
    virtual void columnClicked(int columnId, const ModifierKeys &mods);

private:
    ClipManagerBackend* backend_;
};

class TagTable: public TableListBox,
                public TableListBoxModel,
                public SelectionListener,
                public Timer
{
public:
    TagTable(ClipManagerBackend*);
    ~TagTable(){};
    virtual int getNumRows();
    virtual void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected);
    virtual void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected);
    virtual void cellClicked(int rowNumber, int columnId, const MouseEvent&e);
    virtual void clearSelection();
    void selectCell(String tagName, String tagCategory);
    virtual void timerCallback();
    virtual String getCellTooltip(int rowNumber,int columnId);

protected:
    std::set<int> clickedCells_;
    std::map<int,String>columns_;
    std::map<String,std::vector<String> > tags_;
    int getNumColumns();
    int getCellNumber(int row, int col);
    String getTextForCell(unsigned row, int col);
    String getCategory(int col);
    int getColumn(int);
    int getRow(int);
    int getColumn(String tagCategory);
    XmlElement* getSelectedTags();
    void updateTags();
    void updateHeaders();
    unsigned numRows_;
    ClipManagerBackend* backend_;
    virtual void tagSelectionChanged()=0;
    void selectionChanged();
    int clickedCellNumber_;
    virtual int getDoubleClickTimeout(){return 10;};
    virtual void addEditColumn(int){};
    virtual void setupHeader();

private:
    void doCellClicked(int,int, bool);
    int getCellForTag(String tagName, String tagCategory);    
    int clickRow_;
    int clickCol_;
};

class ClipLibraryTable: public TagTable
{
public:
    ClipLibraryTable(ClipManagerBackend*);
    ~ClipLibraryTable(){};
private:
    virtual void tagSelectionChanged();
};

class ClipEditTable: public TagTable,public TextEditor::Listener
{
public:
    ClipEditTable(ClipManagerBackend*);
    ~ClipEditTable(){};
    void setClip(Clip*);
    virtual void clearSelection();

    virtual Component* refreshComponentForCell(int rowNumber, int columnId,bool isRowSelected, Component* existingComponentToUpdate);
    virtual void textEditorTextChanged(TextEditor &);
    virtual void textEditorReturnKeyPressed(TextEditor &);
    virtual void textEditorEscapeKeyPressed(TextEditor &);
    virtual void textEditorFocusLost(TextEditor &);
    virtual void cellDoubleClicked(int rowNumber, int columnId, const MouseEvent& e);
    virtual void addEditColumn(int);

private:
    virtual void tagSelectionChanged();
    int doubleClickedCellNumber_;
    CellEditor* cellEditor_;
    Clip* clip_;
protected:
    virtual int getDoubleClickTimeout(){return MouseEvent::getDoubleClickTimeout()+10;};
    virtual void setupHeader();
};


#endif
