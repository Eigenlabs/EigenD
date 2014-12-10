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

#ifndef __JUCER_HEADER_AGENTVIEWCOMPONENT_AGENTVIEWCOMPONENT_1E053FD6__
#define __JUCER_HEADER_AGENTVIEWCOMPONENT_AGENTVIEWCOMPONENT_1E053FD6__

//[Headers]     -- You can add your own extra header files here --
#include "Network.h"
#include "juce.h"
#include "TreeTestData.h"
#include "AgentTreeView.h"

class WidgetTabbedComponent;
class AgentViewComponent;

//[/Headers]

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


//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class AgentViewComponent  : public Component,
                            public DragAndDropContainer,
                            public ComponentBoundsConstrainer
{
public:
    //==============================================================================
    AgentViewComponent ();
    ~AgentViewComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.

    void setMainComponent(MainComponent* mainComponent);

    // overloaded functions
    String getSelectedTreeItem();
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);
    bool keyPressed(const KeyPress& key);
    void broughtToFront();

    //==========================================================================
    // XML functions
    void setXMLRPCManager(XMLRPCManager* manager);
    void setTabbedComponent(WidgetTabbedComponent* tabbedComponent);
    void receiveAgentsFromEigenD();
    AgentTreeViewItem* insertAgentInTree(XmlElement* agentXmlElement);
    XmlElement* getElementByPath(const String& path);
    XmlElement* getElementBySubPath(XmlElement* element, const String& path);
    TreeView* getTreeView() { return treeView; }

//    void testAgentView();


    //==============================================================================
    void receiveAgentChanges();
    void clearTree();

    //[/UserMethods]

    void paint (Graphics& g);
    void resized();



private:
    //[UserVariables]   -- You can add your own custom variables in this section.

    MainComponent* mainComponent_;

    XmlElement* treeXml;
    AgentTreeViewItem* rootItem;

    ResizableCornerComponent* resizer_;
    bool dragging_;
    int startX_;
    int startY_;

    XMLRPCManager* XMLRPCManager_;

    int64 agentUpdateTime_;
    String sessionID_;

    WidgetTabbedComponent* tabbedComponent_;

    //[/UserVariables]

    //==============================================================================
    TreeView* treeView;
    Label* label;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AgentViewComponent);
};


#endif   // __JUCER_HEADER_AGENTVIEWCOMPONENT_AGENTVIEWCOMPONENT_1E053FD6__
