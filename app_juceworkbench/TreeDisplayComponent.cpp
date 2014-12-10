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

#include "TreeDisplayComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
TreeDisplayComponent::TreeDisplayComponent (Atom* atom, int nf, int nc)
{

    //[UserPreSize]
    ready_=false;
    treeView_=new TreeView("treeview");
    treeView_ ->setColour(TreeView::linesColourId,Colour(0xffffffff));

    BrowseTreeViewItem* rootItem=new BrowseTreeViewItem(atom,String::empty, nf, nc);
    rootItem->setExpandable(true);
    rootItem->setDisplayText(String("root"));
    treeView_->setRootItem(rootItem);
    addAndMakeVisible(treeView_);
//    int height=680;
//    if((getParentMonitorArea().getHeight())<height)
//    {
//        height=getParentMonitorArea().getHeight()-60;
//    }
    treeView_->setBounds(10,10,380,330);

    //[/UserPreSize]

    setSize (300, 400);


    //[Constructor] You can add your own custom stuff here..

//    setSize (500, height+20);

    rootItem->setOpen(true);
    ready_=true;
    //[/Constructor]
}

TreeDisplayComponent::~TreeDisplayComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]



    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void TreeDisplayComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::black);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void TreeDisplayComponent::resized()
{
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...


void TreeDisplayComponent::enumerate_updated(String path,int nf,int nc)
{
    pic::logmsg()<<"TreeDisplayComponent::enumerate_updated: path= "<<path;
    //TreeViewItem* tvi=treeView_->findItemFromIdentifierString(path.fromFirstOccurrenceOf("/",false,false).replaceCharacter('\\','/'));
    TreeViewItem* tvi=treeView_->findItemFromIdentifierString(path.replaceCharacter('\\','/'));
    if(tvi!=0)
    {
        BrowseTreeViewItem* item=dynamic_cast<BrowseTreeViewItem*>(tvi);
        item->setEnumerate(nf,nc);
    }
    else
    {
        pic::logmsg()<<"      item not found";
    }
}

void TreeDisplayComponent::finfo_updated(String path, const std::set<std::string>& finfo)
{
//    pic::logmsg()<<"TreeDisplayComponent: finfo_updated "<<std::string(path.toUTF8());
    if (path=="/root")
    {
        BrowseTreeViewItem* rootItem=dynamic_cast<BrowseTreeViewItem*> (treeView_->getRootItem());
        if(rootItem!=0)
        {
            rootItem->fillChildFinfo(finfo);
        }
    }
    else
    {
        BrowseTreeViewItem* item=dynamic_cast<BrowseTreeViewItem*>(treeView_->findItemFromIdentifierString(path.replaceCharacter('\\','/')));
        item->fillChildFinfo(finfo);
    }
}

void TreeDisplayComponent::cinfo_updated(String path,const std::set<std::string>& cinfo)
{
    if(path=="/root")
    {
        BrowseTreeViewItem* rootItem=dynamic_cast<BrowseTreeViewItem*>(treeView_->getRootItem());
        rootItem->fillChildCinfo(cinfo);
    }
    else
    {
        //BrowseTreeViewItem* item=dynamic_cast<BrowseTreeViewItem*>(treeView_->findItemFromIdentifierString(path.fromFirstOccurrenceOf("/",false,false).replaceCharacter('\\','/')));
        BrowseTreeViewItem* item=dynamic_cast<BrowseTreeViewItem*>(treeView_->findItemFromIdentifierString(path.replaceCharacter('\\','/')));
        item->fillChildCinfo(cinfo);
    }
}

void TreeDisplayComponent::parentSizeChanged()
{
    if(ready_)
    {
        int ph=getParentComponent()->getHeight();
        int pw=getParentComponent()->getWidth();

        //std::cout<<"parentSizeChanged parent w="<<pw<<" parent h="<<ph<<"  w="<<getWidth()<<" h="<<getHeight()<<std::endl;

        int h=ph-28;
        int w=pw-2;
        setSize(w,h);
        treeView_->setBounds(10,10,w-20,h-20);
    }
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="TreeDisplayComponent" componentName=""
                 parentClasses="public Component" constructorParams="Atom* atom, int nf, int nc"
                 variableInitialisers="" snapPixels="8" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="0" initialWidth="300" initialHeight="400">
  <BACKGROUND backgroundColour="ff000000"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
