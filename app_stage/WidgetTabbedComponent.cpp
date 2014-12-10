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
#include "MainComponent.h"
#include "EigenDialogWindow.h"
#include "DialogFrameworkComponent.h"
#include "DeleteDialogComponent.h"
#include "TabDialogComponent.h"
#include <signal.h>


//==============================================================================

WidgetTabbedComponent::WidgetTabbedComponent(const TabbedButtonBar2::Orientation orientation)
    : TabbedComponent2(orientation), editMode_(true), greyOutsideCanvas_(true),
    doNotShowDeleteConfirm_(false)
{
    // listen to the tab bar add tab button
    getTabbedButtonBar().getAddTabButton()->addMouseListener(this, false);

#if STAGE_BUILD==DESKTOP
    // TODO: remember the last file location used between sessions
    xmlFile = File::getSpecialLocation (File::userHomeDirectory);
    currentDirectory_ = File::getSpecialLocation (File::userHomeDirectory);
#endif // STAGE_BUILD==DESKTOP
}

WidgetTabbedComponent::~WidgetTabbedComponent()
{   
    
    int numTabs = getNumTabs();
    
    // make sure all widget views are deleted
    for (int i=0; i<numTabs; i++) 
    {
        WidgetViewComponent* widgetView = dynamic_cast<WidgetViewComponent*> (getTabContentComponent(getCurrentTabIndex()));
        removeTab(getCurrentTabIndex());
        delete widgetView;
    }
}



//==============================================================================
//
// mode functions
//
//==============================================================================

void WidgetTabbedComponent::setEditMode(const bool editMode)
{
    editMode_ = editMode;

    // set editing mode for content components of all the tabs
    for (int i=0; i<getNumTabs(); i++)
    {
        WidgetViewComponent* widgetView = ((WidgetViewComponent*)getTabContentComponent(i));
        
        widgetView->setEditMode(editMode);

    }
}

void WidgetTabbedComponent::setToolMode(CommandID toolMode, const MouseCursor& editMouseCursor)
{
    toolMode_ = toolMode;
    editMouseCursor_ = editMouseCursor;
    
    // set tool mode for content components of all the tabs
    
    TabbedButtonBar2* tabButtonBar = &getTabbedButtonBar();

    for (int i=0; i<getNumTabs(); i++)
    {
        WidgetViewComponent* widgetView = ((WidgetViewComponent*)getTabContentComponent(i));
        
        widgetView->setToolMode(toolMode_, editMouseCursor_);

/*
        if(!showAddCloseButtons_)
            if(toolMode_==MainComponent::toolCreate ||
               toolMode_==MainComponent::toolEdit ||
               toolMode_==MainComponent::toolDelete)
                tabButtonBar->getTabButton(i)->setMouseCursor(editMouseCursor);
        else
*/
        tabButtonBar->getTabButton(i)->setMouseCursor(MouseCursor::NormalCursor);
        
    }
}

void WidgetTabbedComponent::toggleGreyOutsideCanvas()
{

    setGreyOutsideCanvas(!greyOutsideCanvas_);
    
}

void WidgetTabbedComponent::setGreyOutsideCanvas(bool greyOutsideCanvas)
{    
    
    greyOutsideCanvas_ = greyOutsideCanvas;
    
    MainComponent *mainComponent = findParentComponentOfClass<MainComponent>();
    
    mainComponent->preferences_->setValue("greyOutsideCanvas", greyOutsideCanvas_);
    mainComponent->preferences_->saveIfNeeded();
    
    // set editing mode for content components of all the tabs
    for (int i=0; i<getNumTabs(); i++)
    {
        WidgetViewComponent* widgetView = ((WidgetViewComponent*)getTabContentComponent(i));
        
        widgetView->setGreyOutsideCanvas(greyOutsideCanvas_);
        
    }
}

bool WidgetTabbedComponent::getGreyOutsideCanvas()
{
    return greyOutsideCanvas_;
}

void WidgetTabbedComponent::setDisplayOrientation(Desktop::DisplayOrientation orientation)
{
    MainComponent *mainComponent = findParentComponentOfClass<MainComponent>();
    mainComponent->preferences_->setValue("orientation", orientation);
    mainComponent->preferences_->saveIfNeeded();
}


void WidgetTabbedComponent::setShowAddButton(bool show)
{
    showAddButton_ = show;
    getTabbedButtonBar().setShowAddButton(showAddButton_);
}

void WidgetTabbedComponent::setShowCloseButtons(bool show)
{
    showCloseButtons_ = show;
    
    MainComponent *mainComponent = findParentComponentOfClass<MainComponent>();
    
    mainComponent->preferences_->setValue("showCloseButtons", showCloseButtons_);
    mainComponent->preferences_->saveIfNeeded();
    
    getTabbedButtonBar().setShowCloseButtons(showCloseButtons_);
}

bool WidgetTabbedComponent::getShowCloseButtons()
{
    return showCloseButtons_;
}

void WidgetTabbedComponent::updateAddCloseButtons()
{
    MainComponent *mainComponent = findParentComponentOfClass<MainComponent>();
    
    getTabbedButtonBar().hideAddCloseButtons(mainComponent->getLockMode());
}

void WidgetTabbedComponent::setRightMargin(int newMargin)
{
    // set the space at the right of the tabs before the extra tabs button
    getTabbedButtonBar().setRightMargin(newMargin);
}

void WidgetTabbedComponent::setTabVisible(int tabIndex, bool isVisible)
{
    // set tab visible in tab button bar
    getTabbedButtonBar().setTabVisible(tabIndex, isVisible);
}

void WidgetTabbedComponent::setTabVisibleXml(int tabIndex, bool isVisible)
{
    // set tab visible in tab xml

    // store visibility state in xml for this host
    MainComponent *mainComponent = findParentComponentOfClass<MainComponent>();
    OSCManager* osc = mainComponent->getOSCManager();
    if (osc) 
    {
        String hostname = osc->getHostName();

        if (hostname!="") 
        {
            WidgetViewComponent* widgetView = dynamic_cast<WidgetViewComponent*> (getTabContentComponent(tabIndex));
            XmlElement* xml = widgetView->getXml();
            
            XmlElement* hosts = xml->getChildByName("hosts");
            
            if (hosts)
            {
                for (int i=0; i<hosts->getNumChildElements(); i++) 
                {
                    if (hosts->getChildElement(i)->getStringAttribute("name")==hostname)
                    {
                        hosts->getChildElement(i)->setAttribute("visible", isVisible);

                        // store visibility of this host in eigenD
                        widgetView->setTabInEigenD();
                        return;
                    }
                }
            }
            else 
            {
                xml->addChildElement(hosts = new XmlElement("hosts"));
            }
            XmlElement* host = new XmlElement("host");
            host->setAttribute("name", hostname);
            host->setAttribute("visible", isVisible);
            hosts->addChildElement(host);
            
            // store visibility of this host in eigenD
            widgetView->setTabInEigenD();
        }
    }

}

bool WidgetTabbedComponent::getTabVisible(int tabIndex)
{
    return getTabbedButtonBar().getTabVisible(tabIndex);
}

bool WidgetTabbedComponent::getTabVisibleXml(int tabIndex)
{
    // get custom visibility from xml, otherwise return current visibility settings
    
    MainComponent *mainComponent = findParentComponentOfClass<MainComponent>();
    OSCManager* osc = mainComponent->getOSCManager();
    if (osc)
    {
        String hostname = osc->getHostName();
        
        if (hostname!="") 
        {
            WidgetViewComponent* widgetView = dynamic_cast<WidgetViewComponent*> (getTabContentComponent(tabIndex));
            XmlElement* xml = widgetView->getXml();
            
            XmlElement* hosts = xml->getChildByName("hosts");
            
            if (hosts)
                for (int i=0; i<hosts->getNumChildElements(); i++) 
                    if (hosts->getChildElement(i)->getStringAttribute("name")==hostname)
                        return hosts->getChildElement(i)->getBoolAttribute("visible", true);
            
        }
    }
    
    return getTabbedButtonBar().getTabVisible(tabIndex);
}

//==============================================================================
//
// tab functions
//
//==============================================================================


void WidgetTabbedComponent::initTab(const String &tabName)
{
    // initialize empty tab when receiving tabs from eigenD
    // ready to be filled with widgets

    int tabIndex = getNumTabs();
    
    MainComponent *mainComponent = findParentComponentOfClass<MainComponent>();
    
    WidgetViewComponent* widgetView = new WidgetViewComponent(tabName, tabIndex, 4, 3, 1,
                                                              editMode_, toolMode_, 
                                                              greyOutsideCanvas_, editMouseCursor_, 
                                                              mainComponent, this);
    
    addTab(tabName, Colour((uint8)0xf7, (uint8)0xf7, (uint8)0xf7, (uint8)0xff), widgetView, false);
}

void WidgetTabbedComponent::userAddTab(const String &tabName, float canvasWidthAspect, float canvasHeightAspect, int canvasAspectMode)
{
    // create a new tab
    
    // add tab to end
    int tabIndex = getNumTabs();
   
    MainComponent *mainComponent = findParentComponentOfClass<MainComponent>();

    WidgetViewComponent* widgetView = new WidgetViewComponent(tabName, tabIndex, canvasWidthAspect, canvasHeightAspect, canvasAspectMode,
                                                              editMode_, toolMode_, 
                                                              greyOutsideCanvas_, editMouseCursor_, 
                                                              mainComponent, this);

    addTab(tabName, Colour((uint8)0xf7, (uint8)0xf7, (uint8)0xf7, (uint8)0xff), widgetView, false, tabIndex);

    
    
    
    // add tab in eigenD
    XmlElement* xml = widgetView->getXml();
    
    XmlRpcValue params, result;
    
    String tabXMLStr = xml->createDocument("",false,false);
    //std::cout << tabXMLStr;
    
    //params[0] = tabIndex;
    params = XMLRPCManager::juceToStdString(tabXMLStr);
    
    try
    {
        XMLRPCManager_->execute("addTab", params, result, true);
    }
    catch (XMLRPCAbortException e) {
        // app is shutting down after an xmlrpc has blocked so bail
        return;
    }
}

void WidgetTabbedComponent::userMoveTab(int currentTabIndex, int newTabIndex)
{
    // move the tab, this happens first in the server, then the local tabs are updated
    
    if (newTabIndex>=0 && newTabIndex<getNumTabs() ) 
    {
        XmlRpcValue params, result;
        
        params[0] = currentTabIndex;
        params[1] = newTabIndex;
        
        try
        {
            XMLRPCManager_->execute("moveTab", params, result, true);
        }
        catch (XMLRPCAbortException e) {
            // app is shutting down after an xmlrpc has blocked so bail
            return;
        }
        
        // swap the local tabs to make the display update faster by removing and inserting the current tab
        WidgetViewComponent *currentWidgetView = (WidgetViewComponent*)getTabContentComponent(currentTabIndex);
        WidgetViewComponent *newWidgetView = (WidgetViewComponent*)getTabContentComponent(newTabIndex);
        
        // pick biggest session changes of the two tabs plus 1 to make sure both update when swapped
        int newSessionChanges = max(currentWidgetView->getSessionChanges(), newWidgetView->getSessionChanges())+1;
        currentWidgetView->setSessionChanges(newSessionChanges);
        newWidgetView->setSessionChanges(newSessionChanges);
        
        String tabName = currentWidgetView->getXml()->getStringAttribute("name");
        
        // removing tab does not delete currentWidgetView in that tab
        removeTab(currentTabIndex);

        // create new tab with copy of currentWidgetView
        addTab(tabName, Colour((uint8)0xf7, (uint8)0xf7, (uint8)0xf7, (uint8)0xff), currentWidgetView, false, newTabIndex);

        // set the tab indices to consequetive numbers
        for (int i=0; i<getNumTabs(); i++)
        {
            ((WidgetViewComponent*)getTabContentComponent(i))->setTabIndex(i);
        }
        
        // switch tabs, seem to need to do both to get it to refresh properly...
        setCurrentTabIndex(currentTabIndex);
        setCurrentTabIndex(newTabIndex);
        
        repaint();
    }
}

void WidgetTabbedComponent::userRemoveTab(const int tabIndex, Component *componentToCentreAround)
{
    int result = 1;
    
    if(!doNotShowDeleteConfirm_)
    {
        // ask user for confirmation
        EigenDialogWindow window("Remove Dialog Window");
        DialogFrameworkComponent framework(&window);
        DeleteDialogComponent content;
        
        content.setText("Are you sure you want to delete this tab?");
        window.setContentNonOwned(&framework, true);
        window.centreAroundComponent(componentToCentreAround, framework.getWidth(), framework.getHeight());
        window.setVisible(true);
        framework.setTitleText("Confirm Tab Delete");
        framework.setContentComponent(&content);
        content.setFocusOrder(&framework);
        
        result = framework.runModalLoop();

        doNotShowDeleteConfirm_ = content.getDoNotShowAgain();
        
    }
    
    if(result!=0)
    {    
        if(getNumTabs()>1)
        {
            WidgetViewComponent* widgetView = dynamic_cast<WidgetViewComponent*> (getTabContentComponent(tabIndex));
            removeTab(tabIndex);
            delete widgetView;
        }
        
        // remove tab in eigenD
        XmlRpcValue params, result;
        
        params = tabIndex;
        
        try {
            XMLRPCManager_->execute("removeTab", params, result, true);
        }
        catch (XMLRPCAbortException e) {
            // app is shutting down after an xmlrpc has blocked so bail
            
            // tidy up anyway
            // set the tab indices to consequetive numbers when a hole left from removing a tab
            for (int i=0; i<getNumTabs(); i++)
                ((WidgetViewComponent*)getTabContentComponent(i))->setTabIndex(i);
            
            return;
        }
        
        // set the tab indices to consequetive numbers when a hole left from removing a tab
        for (int i=0; i<getNumTabs(); i++)
            ((WidgetViewComponent*)getTabContentComponent(i))->setTabIndex(i);
    }
}

void WidgetTabbedComponent::userSetTabProperties(int tabIndex)
{
    MainComponent* mainComponent = findParentComponentOfClass<MainComponent>();
    WidgetViewComponent* content = (WidgetViewComponent*)getCurrentContentComponent();
    
    int aspectMode = 0;
    float aspectWidth = 0;
    float aspectHeight = 0;
    content->getAspect(aspectMode, aspectWidth, aspectHeight);
    
    
    EigenDialogWindow window("Tab Dialog Window");
    DialogFrameworkComponent framework(&window);
    TabDialogComponent dialogContent;
    
    dialogContent.initialize(&framework, getCurrentTabName(), aspectWidth, aspectHeight, aspectMode);
    window.setContentNonOwned(&framework, true);
    window.setSize(320, 328);
    window.centreAroundComponent(mainComponent, framework.getWidth(), framework.getHeight());
    window.setVisible(true);
    framework.setTitleText("Tab Properties");
    framework.setContentComponent(&dialogContent);
    dialogContent.setFocusOrder(&framework);
    
    int result = framework.runModalLoop();
    
    if(result!=0)
    {
        content->incSessionChanges();
        
        String newName = dialogContent.getName();
        setTabName(tabIndex, newName);
        content->setTabNameXml(newName);
        getTabbedButtonBar().getTabButton(tabIndex)->setTooltip(newName);
        
        
        aspectMode = dialogContent.getAspectMode();
        
        switch (aspectMode) 
        {
            case 1:
                content->setAspect(1, 4, 3);
                break;
            case 2:
                content->setAspect(2, 4, 1.5);
                break;
            case 3:
                content->setAspect(3, 2, 3);
                break;
            case 4:
                content->setAspect(4, 16, 9);
                break;
            case 5:
                content->setAspect(5, 16, 4.5);
                break;
            case 6:
                content->setAspect(6, 8, 9);
                break;
            // iPhone
            case 7:
                content->setAspect(7, 12, 7);
                break;
            case 8:
                content->setAspect(8, 5, 7);
                break;
            // iPhone 4 Inch
            case 9:
                content->setAspect(9, 2, 1);
                break;
            case 10:
                content->setAspect(10, 3, 5);
                break;
            // iPad
            case 11:
                content->setAspect(11, 7, 5);
                break;
            case 12:
                content->setAspect(12, 11, 14);
                break;
            case 13:
                content->setAspect(13, dialogContent.getWidthAspect(), 
                                   dialogContent.getHeightAspect());
                break;
        }                    
        
    }
    
}

//==============================================================================
//
// overriden component functions
//
//==============================================================================

void WidgetTabbedComponent::currentTabChanged(const int newCurrentTabIndex, const String &newCurrentTabName)
{
    TabbedComponent2::currentTabChanged(newCurrentTabIndex, newCurrentTabName);
    setEditMode(editMode_);
}

TabBarButton2* WidgetTabbedComponent::createTabButton(const String &tabName, const int tabIndex)
{
    // TODO: how is this destructed?
    TabBarButton2* button = new TabBarButton2(tabName, &getTabbedButtonBar(), tabIndex);
    
    // handle mouse events for the buttons in this component
    button->addMouseListener(this, true);

    /*
    if(!showAddCloseButtons_)
        button->setMouseCursor(editMouseCursor_);
     */
    
    // set tooltip to name so it can be viewed even if the tab is too short for the text
    button->setTooltip(tabName);
    
    return button;
}

void WidgetTabbedComponent::mouseDown(const MouseEvent &e)
{
    MainComponent* mainComponent = findParentComponentOfClass<MainComponent>();
    WidgetViewComponent* content = (WidgetViewComponent*)getCurrentContentComponent();

    int tabIndex = 0;
    if(mainComponent->getToolMode()==MainComponent::toolMulti)
        tabIndex = ((TabBarButton2*)e.eventComponent)->getTabIndex();
    else
        tabIndex = getCurrentTabIndex();



    // TODO: determine type of component properly
    // show menu when unlocked
    if((e.mods.isPopupMenu() && e.eventComponent!=this && !mainComponent->getLockMode()
       && mainComponent->getToolMode()==MainComponent::toolMulti) ||
       (e.mods.isLeftButtonDown() && e.eventComponent==content->getCanvasComponent() && mainComponent->getToolMode()==MainComponent::toolEdit)
       )
    {
        if(mainComponent->getToolMode()==MainComponent::toolMulti)
            setCurrentTabIndex(tabIndex);

        // pop up a menu
        PopupMenu m;
        // add 1 to item return codes as 0 is nothing selected
        m.addItem(1, "New Tab", editMode_, false);
        m.addItem(2, "Properties...", editMode_, false);
        m.addSeparator();
        m.addItem(3, "Move Tab Left", editMode_&&(tabIndex>0), false);
        m.addItem(4, "Move Tab Right", editMode_&&(tabIndex<getNumTabs()-1), false);
        m.addSeparator();
        m.addItem (5, "Close Tab", (getNumTabs()>1 && editMode_), false);
        
        int result = m.showAt(juce::Rectangle<int>(e.getScreenX(), e.getScreenY(),1,1));
        
        switch (result) 
        {
            case 1:
                userAddTab("untitled");
                break;
            case 2:
                userSetTabProperties(tabIndex);
                break;
            case 3:
                userMoveTab(tabIndex, tabIndex-1);
                break;
            case 4:
                userMoveTab(tabIndex, tabIndex+1);
                break;
            case 5:
                userRemoveTab(tabIndex, mainComponent);
                break;
        }
    }
    else if(e.mods.isLeftButtonDown() && e.eventComponent!=this)
    {
        if(e.eventComponent==getTabbedButtonBar().getAddTabButton())
        {
            if(showAddButton_)
            {
                // handle tab add button
                userAddTab("untitled");
            }
        }
        else
        {
            if(showCloseButtons_)
            {
                // handle tab close button
                for(int i=0; i<getNumTabs(); i++)
                {
                    Button* closeButton = dynamic_cast<TabBarButton2*>(getTabbedButtonBar().getTabButton(i))->getCloseButton();
                    if(e.eventComponent==closeButton)
                    {
                        userRemoveTab(i, mainComponent);		
                        break;
                    }
                }
                
            }
        }
/*    
        if(!showAddCloseButtons_)
        {
            setCurrentTabIndex(tabIndex);
            
            switch (mainComponent->getToolMode()) {
                case MainComponent::toolCreate:
                    userAddTab("untitled");
                    break;
    //            case MainComponent::toolEdit:
    //                userSetTabName(tabIndex, e.eventComponent);
    //                break;
                case MainComponent::toolDelete:
                    userRemoveTab(tabIndex);
                    break;
                default:
                    break;
            }
        }
*/        
    }
    else
    {
        TabbedComponent2::mouseDown(e);
    }
}



//==============================================================================
//
// XML management functions
//
//==============================================================================

void WidgetTabbedComponent::setXMLRPCManager(XMLRPCManager* manager)
{
    XMLRPCManager_ = manager;
}

void WidgetTabbedComponent::clearTabs()
{
    // messages come from network manager
    // to indicate that the state has changed
    
    // clear all tabs ready for new tabs to be loaded from state
    int numTabs = getNumTabs();
    
    for (int i=0; i<numTabs; i++) 
    {
        WidgetViewComponent* widgetView = dynamic_cast<WidgetViewComponent*> (getTabContentComponent(getCurrentTabIndex()));
        removeTab(getCurrentTabIndex());
        delete widgetView;
    }

    initTab("untitled");
}

void WidgetTabbedComponent::receiveTabChanges()
{
    // check eigenD for tab updates

    XmlRpcValue params, result;

    params.clear();

    try
    {
        XMLRPCManager_->execute("getSessionID", params, result, true);
        String sessionID = String::fromUTF8(std::string(result).c_str());

        if(sessionID != sessionID_)
        {
            sessionID_ = sessionID;
            clearTabs();
        }

    }
    catch(XMLRPCAbortException e)
    {
        return;
    }
    
    std::map<unsigned,String> newTabXml;
    std::map<unsigned,String> updateTabXml;
    std::map<unsigned,int> serverSessionChanges;
    
    MainComponent* mainComponent = findParentComponentOfClass<MainComponent>();
    StatusDialogComponent statusDialogComponent;
    
    int numTabsInEigenD = 0;
    int numTabsInStage = getNumTabs();
    
    // total number of widgets to receive
    int numWidgetsTotal = 0;
    int numWidgetsReceived = 0;
    int numTabsTotal = 0;
    
    
    try
    {
        params.clear();
        XMLRPCManager_->execute("getNumTabs", params, result, true);

        numTabsInEigenD = int(result);

        //std::cout << "server tabs = " << numTabsInEigenD << " client tabs = " << numTabsInStage << " \n";
        if (numTabsInStage>numTabsInEigenD)
        {
            // more tabs in stage in eigenD so reload them

            // determine total number of widget changes
            for (int i=0; i<numTabsInEigenD; i++) 
            {
                params.clear();
                params[0] = i;
                XMLRPCManager_->execute("getNumWidgets", params, result, true);
                if(int(result)!=-1)
                {
                    numWidgetsTotal += int(result);
                    numTabsTotal++;
                }

                params.clear();
                params = i;
                XMLRPCManager_->execute("getTabSessionChanges", params, result, true);
                if(int(result)!=-1)
                    serverSessionChanges.insert(std::pair<unsigned,int>(i,int(result)));
                else
                    serverSessionChanges.insert(std::pair<unsigned,int>(i,0));
            }
            
            // TODO: make this work on iPhone version
//#if STAGE_BUILD==DESKTOP
            if(numTabsTotal>1)
            {
                if (mainComponent)
                {
                    statusDialogComponent.addToDesktop(0);
                    statusDialogComponent.setTitle("Receiving Tabs");
                    statusDialogComponent.setStatusMessage("Receiving tabs and widgets from the EigenD setup.");
                    statusDialogComponent.setUpPosition(mainComponent,320, 200);
                    statusDialogComponent.showStatusMessage(true);
					statusDialogComponent.showQuitButton(false);
                    statusDialogComponent.showConnectButton(false);
					statusDialogComponent.showProgressBar(true);
                    statusDialogComponent.showHostList(false);
                    statusDialogComponent.showAutoToggleButton(false);
					statusDialogComponent.setVisible(true);
                    statusDialogComponent.toFront(true);
                    ComponentPeer* peer = statusDialogComponent.getPeer();
                    if(peer)
                        peer->performAnyPendingRepaintsNow();
                    MessageManager::getInstance()->runDispatchLoopUntil(10);
                }
            }
//#endif // STAGE_BUILD==DESKTOP            
            
            
            // get tabs from eigenD
            for (int i=0; i<numTabsInEigenD; i++) 
            {
                receiveTabAndWidgets(i, newTabXml, numTabsTotal, numWidgetsTotal, numWidgetsReceived, &statusDialogComponent);
            }
            
            // clear all tabs ready for new tabs
            for (int i=0; i<numTabsInStage; i++)
            {
                WidgetViewComponent* widgetView = dynamic_cast<WidgetViewComponent*> (getTabContentComponent(getCurrentTabIndex()));
                removeTab(getCurrentTabIndex());
                delete widgetView;
            }
            
            if (numTabsInEigenD==0)
                // no tabs in eigenD so create new tab in stage and eigenD
                userAddTab("untitled");
        }    
        else 
        {
            // check eigenD for new tabs or tab changes

            // determine total number of widget changes
            for (int i=0; i<numTabsInEigenD; i++) 
            {
                params.clear();
                params = i;
                XMLRPCManager_->execute("getTabSessionChanges", params, result, true);
                
                // compare changes if no error from server
                if(int(result)!=-1)
                {
                    serverSessionChanges.insert(std::pair<unsigned,int>(i,int(result)));
                    
                    if (i>=numTabsInStage) 
                    {
                        params.clear();
                        params[0] = i;
                        XMLRPCManager_->execute("getNumWidgets", params, result, true);
                        if(int(result)!=-1)
                            numWidgetsTotal += int(result);
                        numTabsTotal++;
                    }
                    else 
                    {
                        // check tab for changes
                        WidgetViewComponent* widgetView = (WidgetViewComponent*)getTabContentComponent(i);
                        
                        //std::cout << "tab[" << i << "] session changes: stage=" << widgetView->getSessionChanges() << " eigend=" << serverSessionChanges[i] << "\n";
                        if(widgetView->getSessionChanges()<serverSessionChanges[i])
                        {
                            params.clear();
                            params[0] = i;
                            XMLRPCManager_->execute("getNumWidgets", params, result, true);
                            if(int(result)!=-1)
                                numWidgetsTotal += int(result);
                            numTabsTotal++;
                            
                        }
                    }
                    
                }
            }

            if(numTabsTotal>1)
            {
                if (mainComponent)
                {
                    statusDialogComponent.addToDesktop(0);
                    statusDialogComponent.setTitle("Receiving Tabs");
                    statusDialogComponent.setStatusMessage("Receiving tabs and widgets from the EigenD setup.");
                    statusDialogComponent.setUpPosition(mainComponent,320, 200);
                    statusDialogComponent.showStatusMessage(true);
					statusDialogComponent.showQuitButton(false);
                    statusDialogComponent.showConnectButton(false);
					statusDialogComponent.showProgressBar(true);
                    statusDialogComponent.showHostList(false);
                    statusDialogComponent.showAutoToggleButton(false);
                    statusDialogComponent.setVisible(true);
                    statusDialogComponent.toFront(true);
                    ComponentPeer* peer = statusDialogComponent.getPeer();
                    if(peer)
                        peer->performAnyPendingRepaintsNow();
                    MessageManager::getInstance()->runDispatchLoopUntil(10);
                }
            }
            
            for (int i=0; i<numTabsInEigenD; i++) 
            {
                
                if (i>=numTabsInStage) 
                {
                    // tab doesn't exist, so bring in tab from eigenD
                    receiveTabAndWidgets(i, newTabXml, numTabsTotal, numWidgetsTotal, numWidgetsReceived, &statusDialogComponent);
                }
                else 
                {
                    // check tab for changes
                    WidgetViewComponent* widgetView = (WidgetViewComponent*)getTabContentComponent(i);
                    
                    //std::cout << "tab[" << i << "] session changes: stage=" << widgetView->getSessionChanges() << " eigend=" << serverSessionChanges[i] << "\n";
                    if(widgetView->getSessionChanges()<serverSessionChanges[i])
                    {
                        // stage is out of date with eigenD so get the tab
                        receiveTabAndWidgets(i, updateTabXml, numTabsTotal, numWidgetsTotal, numWidgetsReceived, &statusDialogComponent);
                    }
                }
                    
            }
            
            
        }        
    }
    catch(XMLRPCAbortException e)
    {
        // error occurred in receiving tabs so exit without applying changes
        return;
    }

    //std::cout << "applying tab changes\n";
    
    // apply changes when all changes have been received

    // update existing tabs
    std::map<unsigned,String>::iterator i;

    for (i=updateTabXml.begin(); i!=updateTabXml.end(); i++) 
    {
        String xmlString = (*i).second;
        
        XmlDocument tabXmlDocument (xmlString);
        XmlElement* tabXmlElement = tabXmlDocument.getDocumentElement(false);
        XmlElement* rootXmlElement = tabXmlDocument.getDocumentElement(false);
        
        tabXmlElement->deleteAllChildElementsWithTagName("widget");
        
        if (!tabXmlElement) {
            std::cout << "error creating tab, no tab xml found\n";
            break;
        }
        String tabName = tabXmlElement->getStringAttribute("name");
        
        std::cout << "updating tab[" << (*i).first << "]=" << tabName << "\n";
        
        setTabName((*i).first, tabName);
        getTabbedButtonBar().getTabButton((*i).first)->setTooltip(tabName);

        // tab visibility
        getTabVisibleFromXml((*i).first, rootXmlElement);
        
        WidgetViewComponent* widgetView = (WidgetViewComponent*)getTabContentComponent((*i).first);
        // copy the xml to the widget view
        // sets the tab xml
        widgetView->setXml(tabXmlElement);
        
        // sets tab name and aspect in xml and gets all widgets
        widgetView->getCanvasFromXml(rootXmlElement);

        widgetView->setSessionChanges(serverSessionChanges[(*i).first]);
        
    }
    
    
    // create new tabs
    std::map<unsigned,String>::iterator j;

    for (j=newTabXml.begin(); j!=newTabXml.end(); j++) 
    {
        String xmlString = (*j).second;
        
        XmlDocument tabXmlDocument (xmlString);
        XmlElement* tabXmlElement = tabXmlDocument.getDocumentElement(false);
        XmlElement* rootXmlElement = tabXmlDocument.getDocumentElement(false);

        tabXmlElement->deleteAllChildElementsWithTagName("widget");

        if (!tabXmlElement) {
            std::cout << "error creating tab, no tab xml found\n";
            break;
        }
        
        String tabName = tabXmlElement->getStringAttribute("name");

        std::cout << "creating tab[" << (*j).first << "]=" << tabName << "\n";

        initTab(tabName);
        getTabbedButtonBar().getTabButton((*j).first)->setTooltip(tabName);
        
        // tab visibility
        getTabVisibleFromXml((*j).first, rootXmlElement);
        
        // copy the xml to the widget view
        WidgetViewComponent* widgetView = (WidgetViewComponent*)getTabContentComponent((*j).first);
        widgetView->setXml(tabXmlElement);
        
        widgetView->getCanvasFromXml(rootXmlElement);

        widgetView->setSessionChanges(serverSessionChanges[(*j).first]);

    }
    
    statusDialogComponent.removeFromDesktop();
    ensureCurrentTabVisible();
}


void WidgetTabbedComponent::receiveTabAndWidgets(int tabIndex, std::map<unsigned,String> &newTabXml, int numTabsTotal, int numWidgetsTotal, int &numWidgetsReceived, StatusDialogComponent *statusDialogComponent)
{
    // receive tab xml and all widget xml for the tab and combine them

    //std::cout << "getting tab[" << i << "]\n";
    XmlRpcValue params, result;

    // receive tab
    params.clear();
    params = tabIndex;
    XMLRPCManager_->execute("getTab", params, result, true);
    
    String tabXmlStr = XMLRPCManager::stdToJuceString(std::string(result));
    XmlDocument tabXmlDoc(tabXmlStr);
    XmlElement *tabXmlDocElem = tabXmlDoc.getDocumentElement();
    
    // receive widgets
    params.clear();
    params[0] = tabIndex;
    XMLRPCManager_->execute("getNumWidgets", params, result, true);
    int numWidgets = int(result);
    
    for(int i=0; i<numWidgets; i++)
    {
        
        params.clear();
        params[0] = tabIndex;
        params[1] = i;
        XMLRPCManager_->execute("getWidget", params, result, true);
        
        const String widgetXmlStr = XMLRPCManager::stdToJuceString(std::string(result));
        XmlDocument widgetXmlDoc(widgetXmlStr);
        XmlElement* widgetXmlDocElem = widgetXmlDoc.getDocumentElement();
        
        tabXmlDocElem->addChildElement(widgetXmlDocElem);
        
        numWidgetsReceived++;

    }

    if(numTabsTotal>1)
    {
        statusDialogComponent->setProgress((double)numWidgetsReceived/(double)numWidgetsTotal);
        
        //std::cout << "received=" << numWidgetsReceived << " total=" << numWidgetsTotal << "\n";
        statusDialogComponent->getProgressBar()->updateProgress();
        statusDialogComponent->showStatusMessage(true);
        statusDialogComponent->showHostList(false);
        ComponentPeer *peer = statusDialogComponent->getPeer();
        if(peer)
            peer->performAnyPendingRepaintsNow();
        MessageManager::getInstance()->runDispatchLoopUntil(10);
    }

    String xmlStr = tabXmlDocElem->createDocument("", true, false);
    newTabXml.insert(std::pair<unsigned,String>(tabIndex,xmlStr));


}


void WidgetTabbedComponent::getTabVisibleFromXml(int tabIndex, XmlElement* tabXmlElement)
{
    bool isVisible = true;
    // determine if the visibility state is stored in the xml for this host
    MainComponent *mainComponent = findParentComponentOfClass<MainComponent>();
    OSCManager* osc = mainComponent->getOSCManager();

    StagePreferences* preferences = mainComponent->preferences_;
    if (preferences->getBoolValue("customShowTabs", false))
    {
        if (osc)
        {
            String hostname = osc->getHostName();
            
            if (hostname!="") 
            {
                XmlElement* hosts = tabXmlElement->getChildByName("hosts");
                
                if (hosts)
                {
                    for (int i=0; i<hosts->getNumChildElements(); i++) 
                    {
                        if (hosts->getChildElement(i)->getStringAttribute("name")==hostname)
                        {
                            // tab visibility found in tab xml
                            isVisible = hosts->getChildElement(i)->getBoolAttribute("visible", true);
                            getTabbedButtonBar().setTabVisible(tabIndex, isVisible);
                            return;
                        }
                    }
                }
            }
        }
    }

    // by default, show tab according to preferences
    int aspectMode = tabXmlElement->getIntAttribute("aspectMode", 1);
	isVisible = isVisible && isAspectModeVisible(aspectMode);
    
    getTabbedButtonBar().setTabVisible(tabIndex, isVisible);
    
}

bool WidgetTabbedComponent::isAspectModeVisible(int aspectMode)
{
    MainComponent *mainComponent = findParentComponentOfClass<MainComponent>();
	StagePreferences* preferences = mainComponent->preferences_;

	switch (aspectMode) {
        case 1:
            return preferences->getBoolValue("monitor",true);
        case 2:
            return preferences->getBoolValue("halfMonitorHorizontal",true);
        case 3:
            return preferences->getBoolValue("halfMonitorVertical",true);
        case 4:
            return preferences->getBoolValue("widescreenMonitor",true);
        case 5:
            return preferences->getBoolValue("halfWidescreenMonitorHorizontal",true);
        case 6:
            return preferences->getBoolValue("halfWidescreenMonitorVertical",true);
        case 7:
            return preferences->getBoolValue("iPhoneiPodTouchHorizontal",false);
        case 8:
            return preferences->getBoolValue("iPhoneiPodTouchVertical",false);
        case 9:
            return preferences->getBoolValue("iPhoneiPod4InchHorizontal",false);
        case 10:
            return preferences->getBoolValue("iPhoneiPod4InchVertical",false);
        case 11:
            return preferences->getBoolValue("iPadHorizontal",false);
        case 12:
            return preferences->getBoolValue("iPadVertical",false);
        case 13:
            return preferences->getBoolValue("custom",true);
    }
	
	return true;
}

void WidgetTabbedComponent::updateTabVisibilityFromPreferences()
{
	for (int i=0; i<getNumTabs(); i++)
    {
        WidgetViewComponent* widgetView = ((WidgetViewComponent*)getTabContentComponent(i));
		int aspectMode;
        float aspectWidth, aspectHeight;
        widgetView->getAspect(aspectMode, aspectWidth, aspectHeight);
		
		setTabVisible(i, isAspectModeVisible(aspectMode));
    }
	
	int currentIndex = getCurrentTabIndex();
	if (currentIndex >= 0 && !getTabVisible(currentIndex)) 
	{
		ensureCurrentTabVisible();
		repaint();
	}
}

void WidgetTabbedComponent::ensureCurrentTabVisible()
{
    MainComponent *mainComponent = findParentComponentOfClass<MainComponent>();
    StagePreferences* preferences = mainComponent->preferences_;
    if ((!preferences->getBoolValue("allTabsEditMode", true) && !mainComponent->getLockMode()) || mainComponent->getLockMode()) 
    {
        int currentIndex = getCurrentTabIndex();
        if (!getTabVisible(currentIndex)) 
        {
            int index = currentIndex+1<getNumTabs()?currentIndex+1:0;
            while (index!=currentIndex && !getTabVisible(index)) 
                index = index+1<getNumTabs()?index+1:0;
            setCurrentTabIndex(index, false);
        }
    }

}

void WidgetTabbedComponent::updateShowAllTabs()
{
    MainComponent *mainComponent = findParentComponentOfClass<MainComponent>();
    StagePreferences* preferences = mainComponent->preferences_;
    bool lockMode = mainComponent->getLockMode();
    
    getTabbedButtonBar().setShowAllTabs(!lockMode && preferences->getBoolValue("allTabsEditMode", true));
    
}


void WidgetTabbedComponent::clearHelp()
{
    // clear help from a change in agents to ensure widgets pull new help changes from agent tree
    int numTabs = getNumTabs();
    
    for (int i=0; i<numTabs; i++) 
    {
        WidgetViewComponent* widgetView = ((WidgetViewComponent*)getTabContentComponent(i));
        
        widgetView->clearHelp();
        
    }
   
}


#if STAGE_BUILD==DESKTOP

void WidgetTabbedComponent::importTabs()
{
    MainComponent *mainComponent = findParentComponentOfClass<MainComponent>();
    
    File xmlFile(currentDirectory_.getFullPathName());
    
    
    FileChooser fileChooser ("Please select the tabs file you want to import.",
                               xmlFile,
                               "*.els");
    
    if (fileChooser.browseForFileToOpen())
    {
        xmlFile = fileChooser.getResult();
        
        // load in the tabs
        String xmlString = xmlFile.loadFileAsString();
    
        XmlDocument xmlDoc(xmlString);
        XmlElement* xmlDocElem = xmlDoc.getDocumentElement();
        
        if(xmlDocElem==0 || xmlDocElem->getTagName()!="tabs")
        {
            // error parsing file
            mainComponent = findParentComponentOfClass<MainComponent>();
            AlertDialogComponent::openAlertDialog("Error Importing Tabs", "The tab file could not be read. Either the file has been corrupted or is not a valid Stage tabs file.", mainComponent);

            // delete element because of parsing error
            delete xmlDocElem;
            return;
        }

        mainComponent->getAgentViewComponent()->receiveAgentChanges();
        
        // append the imported tabs to the current tabs
        int tabNum = getNumTabs();
        for (int i=0; i<xmlDocElem->getNumChildElements(); i++) 
        {
            XmlElement *tabXml = xmlDocElem->getChildElement(i);

            String tabName = tabXml->getStringAttribute("name");
            float canvasWidthAspect = (float)tabXml->getDoubleAttribute("widthAspect", 4);
            float canvasHeightAspect = (float)tabXml->getDoubleAttribute("heightAspect", 3);
            int canvasAspectMode = tabXml->getIntAttribute("aspectMode", 1);
            
            std::cout << "appending tab[" << i << "]=" << tabName << "\n";
            
            userAddTab(tabName, canvasWidthAspect, canvasHeightAspect, canvasAspectMode);
            getTabbedButtonBar().getTabButton(tabNum)->setTooltip(tabName);
            
            // copy the xml to the widget view
            WidgetViewComponent* widgetView = dynamic_cast<WidgetViewComponent*>(getTabContentComponent(tabNum));

            // create tab xml copy, and remove widgets from the xml as their xml is not stored
            // in the widget view xml, it is stored in widgets themselves
            XmlElement *widgetViewXml = new XmlElement(*tabXml);
            widgetViewXml->deleteAllChildElementsWithTagName("widget");
            widgetView->setXml(widgetViewXml);
            widgetView->getCanvasPropertiesFromXml();
            
            // set visibility of imported tab
            getTabVisibleFromXml(tabNum, widgetViewXml);

            // now add widgets and their xml
            XmlElement* widget = tabXml->getChildByName("widget");
            while (widget) 
            {
                widgetView->addWidgetFromXml(widget);
                widget = widget->getNextElementWithTagName("widget");
            }
            
            tabNum++;
            
        }
        
    }

    currentDirectory_ = File(xmlFile.getParentDirectory());

    ensureCurrentTabVisible();
    
}



void WidgetTabbedComponent::exportTab()
{
    MainComponent *mainComponent = findParentComponentOfClass<MainComponent>();
    
#if WIN32
    String fileName = currentDirectory_.getFullPathName() + "\\" + getCurrentTabName() + ".els";
#else
    String fileName = currentDirectory_.getFullPathName() + "/" + getCurrentTabName() + ".els";
#endif // WIN32
    File xmlFile(fileName);
    
    
    FileChooser fileChooser ("Please select the filename of the tabs file you want to export.",
                             xmlFile,
                             "*.els");
    
    if (fileChooser.browseForFileToSave(true))
    {
        xmlFile = fileChooser.getResult();
        
        // save out the tabs
        
        XmlElement xmlDocElem("tabs");
        xmlDocElem.setAttribute("about", "Eigenlabs Stage Tabs File, release version " + mainComponent->getVersion());
        
        int tabNum = getCurrentTabIndex();
        
        WidgetViewComponent* widgetView = dynamic_cast<WidgetViewComponent*>(getTabContentComponent(tabNum));
        
        // add xml elements for the tab
        XmlElement* widgetViewXml = new XmlElement(*widgetView->getXml());
        xmlDocElem.addChildElement(widgetViewXml);
        
        // add xml elements for the widgets of the tab
        int numWidgets = widgetView->getNumWidgets();
        for(int j=0; j<numWidgets; j++)
        {
            widgetViewXml->addChildElement(new XmlElement(*widgetView->getWidgetXml(j)));
        }
        
        
        String xmlString = xmlDocElem.createDocument("", false, true);
        
        xmlFile.replaceWithText(xmlString);
        
        // deleting xmlDocElement should delete all it's child elements...
        
    }

    currentDirectory_ = File(xmlFile.getParentDirectory());
    
}

void WidgetTabbedComponent::exportTabs()
{
    MainComponent *mainComponent = findParentComponentOfClass<MainComponent>();
    
    // TODO: should use setup name
#if WIN32
    String fileName = currentDirectory_.getFullPathName() + "\\" + "tabs.els";
#else
    String fileName = currentDirectory_.getFullPathName() + "/" + "tabs.els";
#endif // WIN32
    File xmlFile(fileName);
    
    
    FileChooser fileChooser ("Please select the filename of the tabs file you want to export.",
                             xmlFile,
                             "*.els");
    
    if (fileChooser.browseForFileToSave(true))
    {
        xmlFile = fileChooser.getResult();
        
        // save out the tabs

        XmlElement xmlDocElem("tabs");
        xmlDocElem.setAttribute("about", "Eigenlabs Stage Tabs File, release version " + mainComponent->getVersion());
        
        int numTabs = getNumTabs();
        for(int i=0; i<numTabs; i++)
        {
            WidgetViewComponent* widgetView = dynamic_cast<WidgetViewComponent*>(getTabContentComponent(i));
            
            // add xml elements for the tab
            XmlElement* widgetViewXml = new XmlElement(*widgetView->getXml());
            xmlDocElem.addChildElement(widgetViewXml);
            
            
            // add xml elements for the widgets of the tab
            int numWidgets = widgetView->getNumWidgets();
            for(int j=0; j<numWidgets; j++)
            {
                widgetViewXml->addChildElement(new XmlElement(*widgetView->getWidgetXml(j)));
            }
        }

        
        String xmlString = xmlDocElem.createDocument("", false, true);

        xmlFile.replaceWithText(xmlString);

        // deleting xmlDocElement should delete all it's child elements...
    
    }

    currentDirectory_ = File(xmlFile.getParentDirectory());
}


#endif // STAGE_BUILD==DESKTOP

























