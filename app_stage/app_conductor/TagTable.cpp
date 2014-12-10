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

#include "TagTable.h"

CellEditor::CellEditor(String category, String oldText):TextEditor("cellEditor")
{
    category_=category;
    oldText_=oldText;

    if(oldText_.isEmpty())
    {
        setText("New tag",false);
    }
    else
    {
        setText(oldText,false);
    }
    setSelectAllWhenFocused(true);
    setWantsKeyboardFocus(true);
}

String CellEditor::getCategory()
{
    return category_;
}

String CellEditor::getOldText()
{
    return oldText_;
}

void CellEditor::focusGained(FocusChangeType fct)
{
    std::cout<<"focusGained "<<fct<<std::endl;
    TextEditor::focusGained(fct);
}

void CellEditor::focusLost(FocusChangeType fct)
{
    std::cout<<"focusLost "<<fct<<std::endl;
    TextEditor::focusLost(fct);
}

ClipLibraryTable::ClipLibraryTable(ClipManagerBackend* backend):TagTable(backend)
{
    setupHeader();
}

void ClipLibraryTable::tagSelectionChanged()
{
    std::cout<<"ClipLibraryTable tagSelectionChanged"<<std::endl;
    ClipManagerPanel* cmp=findParentComponentOfClass<ClipManagerPanel>();
    if(cmp!=0)
    {
        cmp->tagSelectionChanged(getSelectedTags());
    }
}

ClipEditTable::ClipEditTable(ClipManagerBackend* backend):TagTable(backend)
{
    cellEditor_=0;
    doubleClickedCellNumber_=0;
    clip_=0;
    setupHeader();
}

void ClipEditTable::addEditColumn(int count)
{
//    XXX
//    Additional column to act as button to add new column
//    getHeader().addColumn("+",count,80,80,80,TableHeaderComponent::notSortable);
}

void ClipEditTable::setupHeader()
{
    setHeader(new TagEditTableHeader(backend_));
    getHeader().setPopupMenuActive(true);
    setHeaderHeight(25);
}

void ClipEditTable::clearSelection()
{
    std::cout<<"TagTable clearSelection"<<std::endl;
    clickedCells_.clear();
    repaint();
}

void ClipEditTable::tagSelectionChanged()
{
    std::cout<<"TagEditTable tagSelectionChanged"<<std::endl;
    XmlElement* clipElement=new XmlElement("Clip");
    if(clip_!=0)
    {
        clipElement->setAttribute("uuid",clip_->getId());
        clipElement->setAttribute("name",clip_->getName());
    }
    else
    {
        std::cout<<"clip_=0"<<std::endl;
    }
    clipElement->addChildElement(getSelectedTags());
    backend_->changeTagsForClip(clipElement);
}

Component* ClipEditTable::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate)
{
    if(existingComponentToUpdate!=0)
    {
        delete existingComponentToUpdate;
    }

    int cellNumber=getCellNumber(rowNumber,columnId);

    if(cellNumber==doubleClickedCellNumber_)
    {
        String cellText=getTextForCell(rowNumber,columnId);
        doubleClickedCellNumber_=0;
        cellEditor_= new CellEditor(getCategory(columnId),cellText);

        cellEditor_->addListener(this);
        return cellEditor_;
    }

    return nullptr;
}

void ClipEditTable::cellDoubleClicked(int rowNumber, int columnId,const MouseEvent& e)
{
    if(!((getHeader().getColumnName(columnId)).equalsIgnoreCase("+")))
    {
        std::cout<<"cellDoubleClicked "<<rowNumber<<" "<<columnId<<std::endl;
        stopTimer();
        doubleClickedCellNumber_=getCellNumber(rowNumber,columnId);
        updateContent();
        if(cellEditor_!=0)
        {
            cellEditor_->grabKeyboardFocus();
        }
    }
}

void ClipEditTable::textEditorTextChanged(TextEditor& cellEditor)
{
    std::cout<<"cell editor changed"<<std::endl;
}

void ClipEditTable::textEditorReturnKeyPressed(TextEditor& cellEditor)
{
    CellEditor& ce=dynamic_cast<CellEditor&>(cellEditor);
        std::cout<<"cell editor return key pressed: new tag="<<ce.getText()<<" category="<<ce.getCategory()<<std::endl;

        if(ce.getOldText().isEmpty() && ce.getText().isEmpty())
        {
            updateContent();
        }
        else if(ce.getOldText().isEmpty())
        {
            XmlElement* e=new XmlElement(String("TAG"));
            e->setAttribute("uuid",clip_->getId());
            e->setAttribute("Name",ce.getText());
            e->setAttribute("Category",ce.getCategory());
            updateContent();
            backend_->addTag(e); 
        }
       
        else if(ce.getText().isEmpty())
        {
            XmlElement* e=new XmlElement(String("TAG"));
            e->setAttribute("uuid",clip_->getId());
            e->setAttribute("Name",ce.getOldText());
            e->setAttribute("Category",ce.getCategory());
            updateContent();
            backend_->removeTag(e);
        }
        else
        {
            XmlElement* e=new XmlElement(String("TAG"));
            e->setAttribute("uuid",clip_->getId());
            e->setAttribute("Name",ce.getText());
            e->setAttribute("oldName",ce.getOldText());
            e->setAttribute("Category",ce.getCategory());
            updateContent();
            backend_->changeTag(e); 
        }
}

void ClipEditTable::textEditorEscapeKeyPressed(TextEditor& cellEditor)
{
    std::cout<<"cell editor escape key pressed"<<std::endl;
    updateContent();
}

void ClipEditTable::textEditorFocusLost(TextEditor& cellEditor)
{
    updateContent();
}

void ClipEditTable::setClip(Clip* clip)
{
    clip_=clip;
    XmlElement* e=clip->toXml()->getFirstChildElement();
    while (e!=0)
    {
        if(e->hasTagName("Tag"))
        {
            selectCell(e->getStringAttribute("name"),e->getStringAttribute("category"));
        }
        e=e->getNextElement();
    }
}

TagEditTableHeader::TagEditTableHeader(ClipManagerBackend* backend)
{
    backend_=backend;
}

void TagEditTableHeader::columnClicked(int columnId, const ModifierKeys &mods)
{
    std::cout<<"TagEditTableHeader column clicked: columnId="<<columnId<<std::endl;
    juce::Rectangle<int> r =getColumnPosition(columnId-1);
    std::cout<<"columnPosition rectangle  x="<<r.getX()<<" y="<<r.getY()<<" width="<<r.getWidth()<<"height="<<r.getHeight()<<std::endl;
    if(getColumnName(columnId).equalsIgnoreCase("+"))
    {
        backend_->addCategory();
    }
}


TagTableHeader::TagTableHeader(ClipManagerBackend* backend)
{
    backend_=backend;
}

void TagTableHeader::columnClicked(int columnId, const ModifierKeys &mods)
{
    std::cout<<"TagTableHeader column clicked: columnId="<<columnId<<std::endl;
}

TagTable::TagTable(ClipManagerBackend* backend)
{
    backend_=backend;
    numRows_=1;
    setModel(this);
    backend_->addSelectionListener(this);
}

void TagTable::setupHeader()
{
    setHeader(new TagTableHeader(backend_));
    getHeader().setPopupMenuActive(true);
    setHeaderHeight(25);
}

void TagTable::selectionChanged()
{
    updateTags();
    updateHeaders();
//    clickedCellNumber_=0;
    repaint();
}

int TagTable::getNumRows()
{
    return numRows_;
}

void TagTable::updateHeaders()
{
    columns_.clear();
    getHeader().removeAllColumns();

    XmlElement* e=backend_->getColumnCategories()->getFirstChildElement();
    int count=1;
    while(e!=0)
    {
        if(e->hasTagName("CATEGORY"))
        {
            String category=e->getStringAttribute("name");
            std::cout<<"updateHeaders "<<category<<std::endl;
            columns_.insert(std::pair<int,String>(count,category));
            count++;
        }
        e=e->getNextElement();
    }    

    for(std::map<int,String>::iterator i=columns_.begin();i!=columns_.end();i++)
    {
        getHeader().addColumn(i->second,i->first,80,80,80,TableHeaderComponent::notSortable);
    }

    addEditColumn(count);
}

void TagTable::updateTags()
{
    tags_.clear();
    XmlElement* e=backend_->getAllTags()->getFirstChildElement();
    while(e!=0)
    {
        std::cout<<e->getTagName()<<std::endl;
        if(e->hasTagName("TAG"))
        {
            String tagName=e->getStringAttribute("name");
            String tagCategory=e->getStringAttribute("category");

            std::map<String,std::vector<String> >::iterator i=tags_.find(tagCategory);
            if(i!=tags_.end())
            {
                (i->second).push_back(tagName);
                std::cout<<tagName<<" inserted into existing vector for category "<<tagCategory<<std::endl;
                if(((i->second).size()+1)>numRows_)
                {
                    numRows_=((i->second).size()+1);
                }
            }
            else
            {
                std::vector<String> t;
                t.push_back(tagName);
                tags_.insert(std::pair<String,std::vector<String> >(tagCategory,t)); 
                std::cout<<tagName<<" inserted into new vector for category "<<tagCategory<<std::endl;
            }
        }

        e=e->getNextElement();
    }
}

int TagTable::getNumColumns()
{
    return columns_.size();
}

int TagTable::getCellNumber(int row, int col)
{
    return (row*getNumColumns())+col;
}

int TagTable::getRow(int cellNumber)
{
    return (cellNumber-1)/getNumColumns();    
}

int TagTable::getColumn(int cellNumber)
{

    int c=cellNumber%getNumColumns();
    if (c==0)
    {
        return getNumColumns();
    }
    return c;
}

void TagTable::selectCell(String tagName, String tagCategory)
{
    std::cout<<"TagTable - selectCell for "<<tagName<< "in category "<<tagCategory<<std::endl;
    int n =getCellForTag(tagName, tagCategory);
    std::cout<<"cell found="<<n<<std::endl;
    if(n!=0)
    {
       doCellClicked(getRow(n), getColumn(n),true); 
       repaint();
    }
}

int TagTable::getColumn(String tagCategory)
{
    for(std::map<int,String>::iterator i =columns_.begin();i!=columns_.end();i++)
    {
        if((i->second).equalsIgnoreCase(tagCategory))
        {
            std::cout<<"getColumn for tagCategory "<<tagCategory<<" returning"<<i->first<<std::endl;
            return (i->first);
        }
    }
    std::cout<<"getColumn did not find column for tagCategory "<<tagCategory<<std::endl;
    return 0;
}

int TagTable::getCellForTag(String tagName, String tagCategory)
{
    int col=getColumn(tagCategory);
    std::map<String,std::vector<String> >::iterator i=tags_.find(tagCategory);
    if(i!=tags_.end())
    {
        std::vector<String> v=i->second;
        int row=0;
        for(std::vector<String> ::iterator j=v.begin();j!=v.end();j++)
        {
            if((*j).equalsIgnoreCase(tagName))
            {
                std::cout<<"found tagname: row="<<row<<std::endl;
                return getCellNumber(row,col);
            }
            row++;
        }
    }
    
    return 0;
}

String TagTable::getTextForCell(unsigned rowNumber, int columnId)
{
//    std::cout<<"getTextForCell row="<<rowNumber<<"columnId="<<columnId<<std::endl;
    String category=getCategory(columnId);
    if(category.isNotEmpty())
    {
        std::map<String,std::vector<String> >::iterator iter=tags_.find(category);
        if(iter!=tags_.end())
        {
            if(rowNumber<(iter->second).size())
            {
//                std::cout<<"tag="<<(iter->second)[rowNumber]<<std::endl;
                return (iter->second)[rowNumber];
            }
        }
    }

    return String::empty;
}

String TagTable::getCellTooltip(int rowNumber,int columnId)
{
    return getTextForCell(rowNumber,columnId);
}

String TagTable::getCategory(int columnId)
{
    std::map<int,String>::iterator i=columns_.find(columnId);
    if(i!=columns_.end())
    {
        return (i->second); 
    }
    return String::empty;
}

void TagTable::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
}

void TagTable::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
        std::set<int>::iterator i=clickedCells_.find(getCellNumber(rowNumber,columnId));
        if(i!=clickedCells_.end())
        {
            g.fillAll (Colours::lightgreen);
        }

        g.setColour (Colours::black.withAlpha (0.2f));
        g.drawRect(0,0,width,height);

        g.setColour (Colours::black);
        g.setFont (14.0f);

        String cellText=getTextForCell(rowNumber,columnId);
        if(cellText.isNotEmpty())
        {
            g.drawText (cellText, 2, 0, width - 4, height, Justification::centredLeft, true);
        }
        g.setColour (Colours::black.withAlpha (0.2f));
        g.fillRect (width - 1, 0, 1, height);
}

void TagTable::cellClicked(int rowNumber, int columnId, const MouseEvent&e)
{
    std::cout<<"cellClicked - doubleclick timeout="<<MouseEvent::getDoubleClickTimeout()<<std::endl;
    clickRow_=rowNumber;
    clickCol_=columnId;
    startTimer(getDoubleClickTimeout());
//    startTimer(MouseEvent::getDoubleClickTimeout()+10);
}

void TagTable::timerCallback()
{
    stopTimer();
    if(getTextForCell(clickRow_,clickCol_).isNotEmpty())
    {
        doCellClicked(clickRow_, clickCol_, false);
        tagSelectionChanged();
        repaint();
    }
}

void TagTable::doCellClicked(int rowNumber, int columnId, bool selectOnly)
{
    std::cout<<"doCellClicked"<<std::endl;
    //std::cout<<"rowNumber "<<rowNumber<< "columnId "<<columnId<<std::endl;
    int n=getCellNumber(rowNumber,columnId);
    //std::cout<<"row "<<getRow(n)<< "col "<<getColumn(n)<<std::endl;
    std::set<int>::iterator i=clickedCells_.find(n);
    if(i!=clickedCells_.end())
    {
        if(!selectOnly)
        {
            clickedCells_.erase(n);
        }
    }
    else
    {
        clickedCells_.insert(n);
    }
}

void TagTable::clearSelection()
{
    std::cout<<"TagTable clearSelection"<<std::endl;
    clickedCells_.clear();
    repaint();
    tagSelectionChanged();
}

XmlElement* TagTable::getSelectedTags()
{
    XmlElement* e=new XmlElement(String("TAGS"));
    for(std::set<int>::iterator i=clickedCells_.begin();i!=clickedCells_.end();i++)
    {
//        std::cout<<getTextForCell(getRow(*i),getColumn(*i))<<std::endl;
        XmlElement* c=new XmlElement(String("TAG"));
        c->setAttribute("Name",getTextForCell(getRow(*i),getColumn(*i)));
        c->setAttribute("Category",getCategory(getColumn(*i)));
        e->addChildElement(c);
    }
    return e;
}

