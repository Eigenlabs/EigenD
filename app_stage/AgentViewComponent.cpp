/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  7 Sep 2012 1:14:50pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

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

 AgentViewComponent: Component to hold the tree view of the agents within the
                       setup running in EigenD
*/
//[/Headers]

#include "AgentViewComponent.h"
#include "WidgetComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
#include <set>
#include <stack>
#define TEST_AGENT_TREE 0
//[/MiscUserDefs]

//==============================================================================
AgentViewComponent::AgentViewComponent ()
    : Component ("AgentView"),
      treeView (0),
      label (0)
{
    addAndMakeVisible (treeView = new TreeView ("agentAtomTreeview"));
    treeView->setColour (TreeView::backgroundColourId, Colour (0xffffff));

    addAndMakeVisible (label = new Label ("new label",
                                          "Browse Available Agents"));
    label->setFont (Font (17.2000f, Font::bold));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colour (0xffc8c8c8));
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x0));


    //[UserPreSize]
    mainComponent_ = 0;

    treeXml = 0;
    rootItem = 0;
    resizer_ = new ResizableCornerComponent(this, this);
    addAndMakeVisible(resizer_);
    //[/UserPreSize]

    setSize (304, 744);


    //[Constructor] You can add your own custom stuff here..
    label->addMouseListener(this, true);
    setMinimumWidth(256);
    setMinimumHeight(128);
    //[/Constructor]
}

AgentViewComponent::~AgentViewComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (treeView);
    deleteAndZero (label);


    //[Destructor]. You can add your own custom destruction code here..
    deleteAndZero(resizer_);
    deleteAndZero(treeXml);
    // TODO: ensure tree is fully destructed
    deleteAndZero(rootItem);
    //[/Destructor]
}

//==============================================================================
void AgentViewComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.setColour (Colour (0xcf000000));
    g.fillRoundedRectangle (3.0f, 3.0f, (float) (getWidth() - 6), (float) (getHeight() - 6), 5.0000f);

    g.setColour (Colour (0xffa3a3a3));
    g.drawRoundedRectangle (3.0f, 3.0f, (float) (getWidth() - 6), (float) (getHeight() - 6), 5.0000f, 2.7000f);

    g.setColour (Colour (0xffe9e9e9));
    g.fillRoundedRectangle (26.0f, 63.0f, (float) (getWidth() - 50), (float) (getHeight() - 87), 3.0000f);

    g.setColour (Colour (0xffa3a3a3));
    g.drawRoundedRectangle (26.0f, 63.0f, (float) (getWidth() - 50), (float) (getHeight() - 87), 3.0000f, 2.7000f);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void AgentViewComponent::resized()
{
    treeView->setBounds (32, 72, getWidth() - 64, getHeight() - 104);
    label->setBounds (12, 12, 276, 24);
    //[UserResized] Add your own custom resize handling here..
    resizer_->setBounds(getWidth()-20,getHeight()-20,16,16);
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void AgentViewComponent::setMainComponent(MainComponent* mainComponent)
{
    mainComponent_ = mainComponent;
}

String AgentViewComponent::getSelectedTreeItem()
{
    AgentTreeViewItem* selected = dynamic_cast<AgentTreeViewItem*>(treeView->getSelectedItem(0));
    if(selected!=0)
        return selected->getDragSourceDescription();
        else
            return "";
}
void AgentViewComponent::mouseDrag (const MouseEvent& e)
{
    int dx=0, dy=0;
    if(e.originalComponent!=treeView)
    {
        if (e.mods.isLeftButtonDown())
        {
            if (!dragging_)
            {
                dragging_ = ! e.mouseWasClicked();
                // start dragging_
                if (dragging_)
                {
                    startX_ = getX();
                    startY_ = getY();
                }
            }
            // continue dragging_
            if (dragging_)
            {
                dx = e.getDistanceFromDragStartX();
                dy = e.getDistanceFromDragStartY();
                setTopLeftPosition(startX_ + dx,
                                   startY_ + dy);
            }
        }
    }
}
void AgentViewComponent::mouseUp (const MouseEvent& e)
{
    dragging_ = false;
}
bool AgentViewComponent::keyPressed(const KeyPress& key)
{
	return false;
}
void AgentViewComponent::broughtToFront()
{
}
//==============================================================================
//
// XMLRPC management functions
//
//==============================================================================
void AgentViewComponent::setXMLRPCManager(XMLRPCManager* manager)
{
    XMLRPCManager_ = manager;
}
void AgentViewComponent::setTabbedComponent(WidgetTabbedComponent* tabbedComponent)
{
    // store tabbedComponent for updating help in widgets when agents change
    tabbedComponent_ = tabbedComponent;

}
void AgentViewComponent::receiveAgentsFromEigenD()
{
    // initialize tree
    treeXml = new XmlElement("setup");
    rootItem = new AgentTreeViewItem(treeXml);
    treeView->setRootItem(rootItem);
    treeView->setMultiSelectEnabled(false);

    XmlRpcValue params, result;
    // get setup name
    params.clear();
    try {
        XMLRPCManager_->execute("getSetupName", params, result, true );
    }
    catch (XMLRPCAbortException e) {
        // app is shutting down after an xmlrpc has blocked so bail
        return;
    }
    treeXml->setAttribute("name", (XMLRPCManager::stdToJuceString(std::string(result))));
    // get initial agent change set
    agentUpdateTime_ = 0;
    receiveAgentChanges();

    // opening the tree elements parses subtrees in the xml
    rootItem->setOpen(true);
}

//void AgentViewComponent::testAgentView()
//{
//    const String pico1XmlString(TreeTestData::pico1_xml);
//    XmlDocument pico1XmlDocument(pico1XmlString);
//    //const String alpha1XmlString(TreeTestData::alpha1_xml);
//    //XmlDocument alpha1XmlDocument(alpha1XmlString);
//    // parse the xml document to get the root xml element
//    treeXml = pico1XmlDocument.getDocumentElement();
//
//    rootItem = new AgentTreeViewItem(treeXml);
//    treeView->setRootItem(rootItem);
//    treeView->setMultiSelectEnabled(false);
//    treeXml->setAttribute("name", "pico 1");
//
//    rootItem->setOpen(true);
//}

void AgentViewComponent::receiveAgentChanges()
{

    // poll eigenD for changes to the agent list
    /*if (XMLRPCManager_->getNetworkDialog()->isCurrentlyModal()) {
        return;
    }*/

    int changeSetSize = 0;
    XmlElement* agentChangeSetElem;
    XmlRpcValue params, result;

    StatusDialogComponent *statusDialogComponent = new StatusDialogComponent;

    try
    {
        params.clear();
        XMLRPCManager_->execute("getSessionID", params, result, true);
        String sessionID = XMLRPCManager::stdToJuceString(std::string(result));

        if(sessionID!=sessionID_)
        {
            //std::cout << "session ID changed in AgentViewComponent\n";
            sessionID_ = sessionID;

            // delete agent tree
            if(treeView)
                treeView->setRootItem(0);
            if(rootItem)
                delete rootItem;
            if(treeXml)
                delete treeXml;

            treeXml = new XmlElement("setup");
            rootItem = new AgentTreeViewItem(treeXml);
            treeView->setRootItem(rootItem);

            // TODO: handle getting this in the change set in the timer callback
            treeXml->setAttribute("name", "setup");

            rootItem->setOpen(true);

            agentUpdateTime_ = 0;
        }

        // get changes to agents
        // TODO: getting this change set can take a long time in eigenD, add progress bar
        params.clear();
        //std::cout << "getAgentChangeSet stage timeStr = " << XMLRPCManager::juceToStdString(String(agentUpdateTime_)) << "\n";
        params = XMLRPCManager::juceToStdString(String(agentUpdateTime_));
        XMLRPCManager_->execute("getAgentChangeSet", params, result, true);

        String agentChangeSetXml = XMLRPCManager::stdToJuceString(std::string(result));
        if(agentChangeSetXml=="")
            return;
        XmlDocument agentChangeSetDoc(agentChangeSetXml);
        agentChangeSetElem = agentChangeSetDoc.getDocumentElement();
        String timeStr = agentChangeSetElem->getStringAttribute("time");
        int64 time = timeStr.getLargeIntValue();
        if (time!=0)
        {
            if(time!=agentUpdateTime_)
            {
                //std::cout << "database timeStr = " << timeStr << " time = " << time << "\n";
                agentUpdateTime_ = time;
            }
        }
        else
        {
            return;
        }

        changeSetSize = agentChangeSetElem->getNumChildElements();

        if(changeSetSize>0)
        {
            //std::cout << "getting agent change set size=" << changeSetSize << "\n";
            // clear all widget help
            tabbedComponent_->clearHelp();
            if(changeSetSize>8)
            {
				mainComponent_->toFront(true);
#if STAGE_BUILD==DESKTOP
				mainComponent_->getMainWindow()->toFront(true);
#endif // STAGE_BUILD==DESKTOP
				statusDialogComponent->addToDesktop(0);
				MessageManager::getInstance()->runDispatchLoopUntil(20);
                statusDialogComponent->setTitle("Receiving Agents");
                statusDialogComponent->setStatusMessage("Receiving agents from the EigenD setup.");
                statusDialogComponent->setUpPosition(mainComponent_,320, 200);
                statusDialogComponent->showStatusMessage(true);
				statusDialogComponent->showQuitButton(false);
                statusDialogComponent->showConnectButton(false);
				statusDialogComponent->showProgressBar(true);
                statusDialogComponent->showHostList(false);
                statusDialogComponent->showAutoToggleButton(false);
				statusDialogComponent->setVisible(true);
				statusDialogComponent->toFront(true);
                ComponentPeer* peer = statusDialogComponent->getPeer();
                if(peer)
                    peer->performAnyPendingRepaintsNow();
				MessageManager::getInstance()->runDispatchLoopUntil(10);
            }
        }
        int changeSetSize = agentChangeSetElem->getNumChildElements();

        for(int i=0; i<changeSetSize; i++)
        {
            if(changeSetSize>8)
            {
                statusDialogComponent->setProgress(((double)i)/((double)changeSetSize));
                //std::cout << "progress = " << (double)i/(double)changeSetSize << "\n";
                statusDialogComponent->getProgressBar()->updateProgress();
                statusDialogComponent->toFront(true);
                ComponentPeer* peer = statusDialogComponent->getPeer();
                if(peer)
                    peer->performAnyPendingRepaintsNow();
            }

            String changeId = agentChangeSetElem->getChildElement(i)->getStringAttribute("address");

            params.clear();
            params = XMLRPCManager::juceToStdString(changeId);
            //params[1] = true;
            XMLRPCManager_->execute("getAgent", params, result, true);

            //std::cout << "getting agent " << changeId << "\n";

            String agentXmlString = XMLRPCManager::stdToJuceString(std::string(result));
            //std::cout << "received agent:\n" << agentXmlString << "\n";

            XmlDocument agentXmlDocument (agentXmlString);
            XmlElement* agentXmlElement = agentXmlDocument.getDocumentElement();

            // find agent in the tree by id
            bool createAgent = true;

            for(int j=0; j<rootItem->getNumSubItems(); j++)
            {
                //std::cout << "id = " << changeId << " addr=" << treeXml->getChildElement(j)->getStringAttribute("address") << "\n";

                AgentTreeViewItem* agent = dynamic_cast<AgentTreeViewItem*>(rootItem->getSubItem(j));

                if (agent->getXml()->getStringAttribute("address")==changeId)
                {
                    if (agentXmlString=="")
                    {
                        // agent is empty so delete agent from tree
                        //std::cout << "delete agent " << changeId << "\n";

                        treeXml->removeChildElement(agent->getXml(), true);
                        rootItem->removeSubItem(j, true);
                    }
                    else
                    {
                        // change this agent
                        //std::cout << "change agent " << changeId << "\n";

                        // preserve the full names of the agents that are open
                        std::set<juce::String> openitems;
                        if (agent->isOpen())
                        {
                            std::stack<std::pair<juce::String, AgentTreeViewItem*> > itemstack;
                            itemstack.push(std::make_pair(juce::String(""), agent));
                            while (!itemstack.empty())
                            {
                                std::pair<juce::String, AgentTreeViewItem*> iteminfo = itemstack.top();
                                itemstack.pop();

                                AgentTreeViewItem* item = iteminfo.second;
                                AgentTreeViewItem* subitem;
                                juce::String fullname = iteminfo.first+juce::String(":")+item->getUniqueName();

                                openitems.insert(fullname);

                                for (int c=0; c<item->getNumSubItems(); c++)
                                {
                                    subitem = dynamic_cast<AgentTreeViewItem*>(item->getSubItem(c));
                                    if(subitem->mightContainSubItems() && subitem->isOpen() && !subitem->getUniqueName().isEmpty())
                                    {
                                        itemstack.push(std::make_pair(fullname, subitem));
                                    }
                                }
                            }
                        }

                        // reinsert according to name (which can change)
                        treeXml->removeChildElement(agent->getXml(), true);

                        rootItem->removeSubItem(j, true);
                        AgentTreeViewItem* newAgent = insertAgentInTree(agentXmlElement);

                        // restore all the open items
                        if (newAgent)
                        {
                            std::stack<std::pair<juce::String, AgentTreeViewItem*> > itemstack;
                            itemstack.push(std::make_pair(juce::String(""), newAgent));
                            while (!itemstack.empty())
                            {
                                std::pair<juce::String, AgentTreeViewItem*> iteminfo = itemstack.top();
                                itemstack.pop();

                                AgentTreeViewItem* item = iteminfo.second;
                                AgentTreeViewItem* subitem;
                                juce::String fullname = iteminfo.first+juce::String(":")+item->getUniqueName();
                                if (openitems.find(fullname) != openitems.end())
                                {
                                    item->setOpen(true);
                                }
                                else
                                {
                                    item->setOpen(false);
                                }

                                for (int c=0; c<item->getNumSubItems(); c++)
                                {
                                    subitem = dynamic_cast<AgentTreeViewItem*>(item->getSubItem(c));
                                    if(subitem->mightContainSubItems() && !subitem->getUniqueName().isEmpty())
                                    {
                                        itemstack.push(std::make_pair(fullname, subitem));
                                    }
                                }
                            }
                        }
                    }

                    rootItem->treeHasChanged();

                    createAgent = false;
                    break;
                }
            }

            if (createAgent)
            {
                //std::cout << "create agent " << changeId << "\n";

                // not in tree so add
                insertAgentInTree(agentXmlElement);
            }
        }

        //statusDialogComponent->setVisible(false);
        statusDialogComponent->removeFromDesktop();

    }
    catch (XMLRPCAbortException e) {
        // app is shutting down after an xmlrpc has blocked so bail
        return;
    }
    // update tree view
    if(changeSetSize>0)
    {
        rootItem->treeHasChanged();
        //std::cout << "done agent change set\n";
    }
    //std::cout << "num sub items=" << rootItem->getNumSubItems() << " numChildElements=" << treeXml->getNumChildElements() << "\n";



}

AgentTreeViewItem* AgentViewComponent::insertAgentInTree(XmlElement* agentXmlElement)
{
    // insert agent in tree according to the alphabetical and ordinal order of the name
    if (!agentXmlElement)
    {
        return 0;
    }

    String agentName = agentXmlElement->getStringAttribute("name");
    // remove the trailing ordinal
    String agentNameNoOrd = agentName.trimCharactersAtEnd("0123456789 ");
    // get the trailing ordinal
    int agentOrdinal = agentName.getTrailingIntValue();

    for(int j=0; j<rootItem->getNumSubItems(); j++)
    {
        AgentTreeViewItem* agent = dynamic_cast<AgentTreeViewItem*>(rootItem->getSubItem(j));

        String treeAgentName = agent->getXml()->getStringAttribute("name");
        String treeAgentNameNoOrd = treeAgentName.trimCharactersAtEnd("0123456789 ");
        int treeAgentOrdinal = treeAgentName.getTrailingIntValue();
        if(treeAgentNameNoOrd>agentNameNoOrd || (treeAgentNameNoOrd==agentNameNoOrd && treeAgentOrdinal>agentOrdinal))
        {
            AgentTreeViewItem* newAgent = new AgentTreeViewItem(agentXmlElement);
            rootItem->addSubItem(newAgent, j);
            treeXml->insertChildElement(agentXmlElement, j);
            return newAgent;
        }
    }

    // if did not insert, then put at end
    AgentTreeViewItem* newAgent = new AgentTreeViewItem(agentXmlElement);
    rootItem->addSubItem(newAgent);
    treeXml->addChildElement(agentXmlElement);

    return newAgent;

}
XmlElement* AgentViewComponent::getElementByPath(const String& path)
{
    if (treeXml)
    {
        // ignore 'setup' root element
        XmlElement* firstChildElement = treeXml->getFirstChildElement();
        if (firstChildElement)
        {
            return getElementBySubPath(firstChildElement, path);
        }
    }
    return 0;
}
XmlElement* AgentViewComponent::getElementBySubPath(XmlElement* element, const String& path)
{
    // get an XML element in the tree according to the OSC path by a recursive tree search
    if(element==0)
        return 0;

    // the name of this level from the path
    String pathHeadName = path.trimCharactersAtStart("/");
    // the remaining path
    String subPath = pathHeadName.fromFirstOccurrenceOf("/", true, false);

    if(subPath!="")
    {
        // strip remaining path from head
        pathHeadName = pathHeadName.upToFirstOccurrenceOf("/", false, false);
    }
    // convert to atom name format from osc format
    pathHeadName = pathHeadName.replace("_", " ");

    // search on first part of path by comparing with name
    String name = element->getStringAttribute("name");
    //std::cout << "name=" << name << "  path head=" << pathHeadName << "  sub path=" << subPath << "\n";
    if(pathHeadName==name)
    {
        if (subPath=="")
        {
            //std::cout << "name=" << element->getChildByName("help")->getTagName() << "\n" << element->getChildByName("help")->getAllSubText() << "\n\n";

            // no subtree so a match
            return element;
        }
        else
        {
            // must search further down tree
            XmlElement* result = getElementBySubPath(element->getFirstChildElement(), subPath);
            if(result)
                return result;
        }
    }
    else
    {
        // search next sibling
        XmlElement* result = getElementBySubPath(element->getNextElement(), path);
        if(result)
            return result;
    }
    return 0;

}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="AgentViewComponent" componentName="AgentView"
                 parentClasses="public Component, public DragAndDropContainer, public ComponentBoundsConstrainer"
                 constructorParams="" variableInitialisers="" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330000013" fixedSize="1" initialWidth="304"
                 initialHeight="744">
  <BACKGROUND backgroundColour="0">
    <ROUNDRECT pos="3 3 6M 6M" cornerSize="5" fill="solid: cf000000" hasStroke="1"
               stroke="2.70000005, mitered, butt" strokeColour="solid: ffa3a3a3"/>
    <ROUNDRECT pos="26 63 50M 87M" cornerSize="3" fill="solid: ffe9e9e9" hasStroke="1"
               stroke="2.70000005, mitered, butt" strokeColour="solid: ffa3a3a3"/>
  </BACKGROUND>
  <TREEVIEW name="agentAtomTreeview" id="5906d04d1bfd8338" memberName="treeView"
            virtualName="" explicitFocusOrder="0" pos="32 72 64M 104M" backgroundColour="ffffff"
            rootVisible="1" openByDefault="0"/>
  <LABEL name="new label" id="d02617478241afef" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="12 12 276 24" textCol="ffc8c8c8"
         edTextCol="ff000000" edBkgCol="0" labelText="Browse Available Agents"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="17.2" bold="1" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
