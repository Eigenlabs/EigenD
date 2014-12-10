/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

//#include "../../../core/juce_StandardHeader.h"

//BEGIN_JUCE_NAMESPACE


#include "TabbedComponent2.h"
//#include "../../graphics/geometry/juce_RectangleList.h"


//==============================================================================
class TabCompButtonBar2  : public TabbedButtonBar2
{
public:
    TabCompButtonBar2 (TabbedComponent2* const owner_,
                      const TabbedButtonBar2::Orientation orientation)
        : TabbedButtonBar2 (orientation),
          owner (owner_)
    {
    }

    ~TabCompButtonBar2()
    {
    }

    void currentTabChanged (const int newCurrentTabIndex,
                            const String& newTabName)
    {
        owner->changeCallback (newCurrentTabIndex, newTabName);
    }

    void popupMenuClickOnTab (const int tabIndex,
                              const String& tabName)
    {
        owner->popupMenuClickOnTab (tabIndex, tabName);
    }

    const Colour getTabBackgroundColour (const int tabIndex)
    {
        return owner->tabs->getTabBackgroundColour (tabIndex);
    }

    TabBarButton2* createTabButton (const String& tabName, const int tabIndex)
    {
        return owner->createTabButton (tabName, tabIndex);
    }

private:
    TabbedComponent2* const owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TabCompButtonBar2);
};


TabbedComponent2::TabbedComponent2 (const TabbedButtonBar2::Orientation orientation)
    : panelComponent (0),
      tabDepth (30),
      outlineThickness (1),
      edgeIndent (0)
{
    addAndMakeVisible (tabs = new TabCompButtonBar2 (this, orientation));
}

TabbedComponent2::~TabbedComponent2()
{
    clearTabs();
    delete tabs;
}

//==============================================================================
void TabbedComponent2::setOrientation (const TabbedButtonBar2::Orientation orientation)
{
    tabs->setOrientation (orientation);
    resized();
}

TabbedButtonBar2::Orientation TabbedComponent2::getOrientation() const throw()
{
    return tabs->getOrientation();
}

void TabbedComponent2::setTabBarDepth (const int newDepth)
{
    if (tabDepth != newDepth)
    {
        tabDepth = newDepth;
        resized();
    }
}

TabBarButton2* TabbedComponent2::createTabButton (const String& tabName, const int tabIndex)
{
    return new TabBarButton2 (tabName, tabs, tabIndex);
}

//==============================================================================
void TabbedComponent2::clearTabs()
{
    if (panelComponent != 0)
    {
        panelComponent->setVisible (false);
        removeChildComponent (panelComponent);
        panelComponent = 0;
    }

    tabs->clearTabs();

    for (int i = contentComponents.size(); --i >= 0;)
    {
        Component* const c = contentComponents.getUnchecked(i);

        // be careful not to delete these components until they've been removed from the tab component
        jassert (c == 0 || c->isValidComponent());

        if (c != 0 && (bool) c->getProperties() ["deleteByTabComp_"])
            delete c;
    }

    contentComponents.clear();
}

void TabbedComponent2::addTab (const String& tabName,
                              const Colour& tabBackgroundColour,
                              Component* const contentComponent,
                              const bool deleteComponentWhenNotNeeded,
                              const int insertIndex)
{
    contentComponents.insert (insertIndex, contentComponent);

    if (contentComponent != 0)
        contentComponent->getProperties().set ("deleteByTabComp_", deleteComponentWhenNotNeeded);

    tabs->addTab (tabName, tabBackgroundColour, insertIndex);
}

void TabbedComponent2::setTabName (const int tabIndex,
                                  const String& newName)
{
    tabs->setTabName (tabIndex, newName);
}

void TabbedComponent2::removeTab (const int tabIndex)
{
    Component* const c = contentComponents [tabIndex];

    if (c != 0 && (bool) c->getProperties() ["deleteByTabComp_"])
    {
        if (c == panelComponent)
            panelComponent = 0;

        delete c;
    }

    contentComponents.remove (tabIndex);

    tabs->removeTab (tabIndex);
}

int TabbedComponent2::getNumTabs() const
{
    return tabs->getNumTabs();
}

const StringArray TabbedComponent2::getTabNames() const
{
    return tabs->getTabNames();
}

Component* TabbedComponent2::getTabContentComponent (const int tabIndex) const throw()
{
    return contentComponents [tabIndex];
}

const Colour TabbedComponent2::getTabBackgroundColour (const int tabIndex) const throw()
{
    return tabs->getTabBackgroundColour (tabIndex);
}

void TabbedComponent2::setTabBackgroundColour (const int tabIndex, const Colour& newColour)
{
    tabs->setTabBackgroundColour (tabIndex, newColour);

    if (getCurrentTabIndex() == tabIndex)
        repaint();
}

void TabbedComponent2::setCurrentTabIndex (const int newTabIndex, const bool sendChangeMessage)
{
    tabs->setCurrentTabIndex (newTabIndex, sendChangeMessage);
}

int TabbedComponent2::getCurrentTabIndex() const
{
    return tabs->getCurrentTabIndex();
}

const String& TabbedComponent2::getCurrentTabName() const
{
    return tabs->getCurrentTabName();
}

void TabbedComponent2::setOutline (int thickness)
{
    outlineThickness = thickness;
    repaint();
}

void TabbedComponent2::setIndent (const int indentThickness)
{
    edgeIndent = indentThickness;
}

void TabbedComponent2::paint (Graphics& g)
{
    g.fillAll (findColour (backgroundColourId));

    const TabbedButtonBar2::Orientation o = getOrientation();

    int x = 0;
    int y = 0;
    int r = getWidth();
    int b = getHeight();

    if (o == TabbedButtonBar2::TabsAtTop)
        y += tabDepth;
    else if (o == TabbedButtonBar2::TabsAtBottom)
        b -= tabDepth;
    else if (o == TabbedButtonBar2::TabsAtLeft)
        x += tabDepth;
    else if (o == TabbedButtonBar2::TabsAtRight)
        r -= tabDepth;

    g.reduceClipRegion (x, y, r - x, b - y);
    g.fillAll (tabs->getTabBackgroundColour (getCurrentTabIndex()));

    if (outlineThickness > 0)
    {
        if (o == TabbedButtonBar2::TabsAtTop)
            --y;
        else if (o == TabbedButtonBar2::TabsAtBottom)
            ++b;
        else if (o == TabbedButtonBar2::TabsAtLeft)
            --x;
        else if (o == TabbedButtonBar2::TabsAtRight)
            ++r;

        g.setColour (findColour (outlineColourId));
        g.drawRect (x, y, r - x, b - y, outlineThickness);
    }
}

void TabbedComponent2::resized()
{
    const TabbedButtonBar2::Orientation o = getOrientation();
    const int indent = edgeIndent + outlineThickness;
    BorderSize<int> indents (indent);

    if (o == TabbedButtonBar2::TabsAtTop)
    {
        tabs->setBounds (0, 0, getWidth(), tabDepth);
        indents.setTop (tabDepth + edgeIndent);
    }
    else if (o == TabbedButtonBar2::TabsAtBottom)
    {
        tabs->setBounds (0, getHeight() - tabDepth, getWidth(), tabDepth);
        indents.setBottom (tabDepth + edgeIndent);
    }
    else if (o == TabbedButtonBar2::TabsAtLeft)
    {
        tabs->setBounds (0, 0, tabDepth, getHeight());
        indents.setLeft (tabDepth + edgeIndent);
    }
    else if (o == TabbedButtonBar2::TabsAtRight)
    {
        tabs->setBounds (getWidth() - tabDepth, 0, tabDepth, getHeight());
        indents.setRight (tabDepth + edgeIndent);
    }

    const Rectangle<int> bounds (indents.subtractedFrom (Rectangle<int> (0, 0, getWidth(), getHeight())));

    for (int i = contentComponents.size(); --i >= 0;)
        if (contentComponents.getUnchecked (i) != 0)
            contentComponents.getUnchecked (i)->setBounds (bounds);
}

void TabbedComponent2::lookAndFeelChanged()
{
    for (int i = contentComponents.size(); --i >= 0;)
        if (contentComponents.getUnchecked (i) != 0)
            contentComponents.getUnchecked (i)->lookAndFeelChanged();
}

void TabbedComponent2::changeCallback (const int newCurrentTabIndex,
                                      const String& newTabName)
{
    if (panelComponent != 0)
    {
        panelComponent->setVisible (false);
        removeChildComponent (panelComponent);
        panelComponent = 0;
    }

    if (getCurrentTabIndex() >= 0)
    {
        panelComponent = contentComponents [getCurrentTabIndex()];

        if (panelComponent != 0)
        {
            // do these ops as two stages instead of addAndMakeVisible() so that the
            // component has always got a parent when it gets the visibilityChanged() callback
            addChildComponent (panelComponent);
            panelComponent->setVisible (true);
            panelComponent->toFront (true);
        }

        repaint();
    }

    resized();

    currentTabChanged (newCurrentTabIndex, newTabName);
}

void TabbedComponent2::currentTabChanged (const int, const String&)
{
}

void TabbedComponent2::popupMenuClickOnTab (const int, const String&)
{
}

//END_JUCE_NAMESPACE
