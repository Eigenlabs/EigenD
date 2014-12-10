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

#include "WBTabbedComponent.h"

namespace TabbedComponentHelpers
{
	const Identifier deleteComponentId ("deleteByTabComp_");

	void deleteIfNecessary (Component* const comp)
	{
		if (comp != nullptr && (bool) comp->getProperties() [deleteComponentId])
			delete comp;
	}

	const juce::Rectangle<int> getTabArea (juce::Rectangle<int>& content, BorderSize<int>& outline,
									 const WBTabbedButtonBar::Orientation orientation, const int tabDepth)
	{
		switch (orientation)
		{
			case WBTabbedButtonBar::TabsAtTop:	outline.setTop (0);	 return content.removeFromTop (tabDepth);
			case WBTabbedButtonBar::TabsAtBottom: outline.setBottom (0);  return content.removeFromBottom (tabDepth);
			case WBTabbedButtonBar::TabsAtLeft:   outline.setLeft (0);	return content.removeFromLeft (tabDepth);
			case WBTabbedButtonBar::TabsAtRight:  outline.setRight (0);   return content.removeFromRight (tabDepth);
			default: jassertfalse; break;
		}

		return juce::Rectangle<int>();
	}
}


class WBTabbedComponent::ButtonBar  : public WBTabbedButtonBar
{
public:
	ButtonBar (WBTabbedComponent& owner_, const WBTabbedButtonBar::Orientation orientation_)
		: WBTabbedButtonBar (orientation_),
		  owner (owner_)
	{
	}

	void currentTabChanged (int newCurrentTabIndex, const String& newTabName)
	{
		owner.changeCallback (newCurrentTabIndex, newTabName);
	}

	void popupMenuClickOnTab (int tabIndex, const String& tabName)
	{
		owner.popupMenuClickOnTab (tabIndex, tabName);
	}
    void closeButtonClicked(int tabIndex, String name)
    {
        owner.requestCloseTab(tabIndex, name);
    }

	const Colour getTabBackgroundColour (const int tabIndex)
	{
		return owner.tabs->getTabBackgroundColour (tabIndex);
	}

	WBTabBarButton* createTabButton (const String& tabName, int tabIndex)
	{
		return owner.createTabButton (tabName, tabIndex);
	}

private:
	WBTabbedComponent& owner;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonBar);
};

WBTabbedComponent::WBTabbedComponent (const WBTabbedButtonBar::Orientation orientation)
	: tabDepth (30),
	  outlineThickness (1),
	  edgeIndent (0),
      wsf_(0)
{
	addAndMakeVisible (tabs = new ButtonBar (*this, orientation));
}

WBTabbedComponent::~WBTabbedComponent()
{
	clearTabs();
	tabs = nullptr;
}

void WBTabbedComponent::addRemoveTabListener(WorkspaceFactory* wsf)
{
    wsf_=wsf;
}

void WBTabbedComponent::mouseDown(const MouseEvent &e)
{
    std::cout<<"WBTabbedComponent mouseDown"<<std::endl;
}

void WBTabbedComponent::setOrientation (const WBTabbedButtonBar::Orientation orientation)
{
	tabs->setOrientation (orientation);
	resized();
}

WBTabbedButtonBar::Orientation WBTabbedComponent::getOrientation() const noexcept
{
	return tabs->getOrientation();
}

void WBTabbedComponent::setTabBarDepth (const int newDepth)
{
	if (tabDepth != newDepth)
	{
		tabDepth = newDepth;
		resized();
	}
}

WBTabBarButton* WBTabbedComponent::createTabButton (const String& tabName, const int /*tabIndex*/)
{
	return new WBTabBarButton (tabName, *tabs);
}

void WBTabbedComponent::clearTabs()
{
	if (panelComponent != nullptr)
	{
		panelComponent->setVisible (false);
		removeChildComponent (panelComponent);
		panelComponent = nullptr;
	}

	tabs->clearTabs();

	for (int i = contentComponents.size(); --i >= 0;)
		TabbedComponentHelpers::deleteIfNecessary (contentComponents.getReference (i));

	contentComponents.clear();
}

void WBTabbedComponent::addTab (const String& tabName,
							  const Colour& tabBackgroundColour,
							  Component* const contentComponent,
							  const bool deleteComponentWhenNotNeeded,
							  const int insertIndex)
{
	contentComponents.insert (insertIndex, WeakReference<Component> (contentComponent));

	if (deleteComponentWhenNotNeeded && contentComponent != nullptr)
		contentComponent->getProperties().set (TabbedComponentHelpers::deleteComponentId, true);

	tabs->addTab (tabName, tabBackgroundColour, insertIndex);
}

void WBTabbedComponent::setTabName (const int tabIndex, const String& newName)
{
	tabs->setTabName (tabIndex, newName);
}

void WBTabbedComponent::removeTab (const int tabIndex)
{
	if (isPositiveAndBelow (tabIndex, contentComponents.size()))
	{
		TabbedComponentHelpers::deleteIfNecessary (contentComponents.getReference (tabIndex));
		contentComponents.remove (tabIndex);
		tabs->removeTab (tabIndex);
	}
}

int WBTabbedComponent::getNumTabs() const
{
	return tabs->getNumTabs();
}

StringArray WBTabbedComponent::getTabNames() const
{
	return tabs->getTabNames();
}

Component* WBTabbedComponent::getTabContentComponent (const int tabIndex) const noexcept
{
	return contentComponents [tabIndex];
}

const Colour WBTabbedComponent::getTabBackgroundColour (const int tabIndex) const noexcept
{
	return tabs->getTabBackgroundColour (tabIndex);
}

void WBTabbedComponent::setTabBackgroundColour (const int tabIndex, const Colour& newColour)
{
	tabs->setTabBackgroundColour (tabIndex, newColour);

	if (getCurrentTabIndex() == tabIndex)
		repaint();
}

void WBTabbedComponent::setCurrentTabIndex (const int newTabIndex, const bool sendChangeMessage)
{
	tabs->setCurrentTabIndex (newTabIndex, sendChangeMessage);
}

int WBTabbedComponent::getCurrentTabIndex() const
{
	return tabs->getCurrentTabIndex();
}

String WBTabbedComponent::getCurrentTabName() const
{
	return tabs->getCurrentTabName();
}

void WBTabbedComponent::setOutline (const int thickness)
{
	outlineThickness = thickness;
	resized();
	repaint();
}

void WBTabbedComponent::setIndent (const int indentThickness)
{
	edgeIndent = indentThickness;
	resized();
	repaint();
}

void WBTabbedComponent::paint (Graphics& g)
{
	g.fillAll (findColour (backgroundColourId));

	juce::Rectangle<int> content (getLocalBounds());
	BorderSize<int> outline (outlineThickness);
	TabbedComponentHelpers::getTabArea (content, outline, getOrientation(), tabDepth);

	g.reduceClipRegion (content);
	g.fillAll (tabs->getTabBackgroundColour (getCurrentTabIndex()));

	if (outlineThickness > 0)
	{
		RectangleList<int> rl (content);
		rl.subtract (outline.subtractedFrom (content));

		g.reduceClipRegion (rl);
		g.fillAll (findColour (outlineColourId));
	}
}

void WBTabbedComponent::resized()
{
	juce::Rectangle<int> content (getLocalBounds());
	BorderSize<int> outline (outlineThickness);

	tabs->setBounds (TabbedComponentHelpers::getTabArea (content, outline, getOrientation(), tabDepth));
	content = BorderSize<int> (edgeIndent).subtractedFrom (outline.subtractedFrom (content));

	for (int i = contentComponents.size(); --i >= 0;)
		if (contentComponents.getReference (i) != nullptr)
			contentComponents.getReference (i)->setBounds (content);
}

void WBTabbedComponent::lookAndFeelChanged()
{
	for (int i = contentComponents.size(); --i >= 0;)
		if (contentComponents.getReference (i) != nullptr)
			contentComponents.getReference (i)->lookAndFeelChanged();
}

void WBTabbedComponent::changeCallback (const int newCurrentTabIndex, const String& newTabName)
{
	if (panelComponent != nullptr)
	{
		panelComponent->setVisible (false);
		removeChildComponent (panelComponent);
		panelComponent = nullptr;
	}

	if (getCurrentTabIndex() >= 0)
	{
		panelComponent = getTabContentComponent (getCurrentTabIndex());

		if (panelComponent != nullptr)
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

void WBTabbedComponent::currentTabChanged (const int, const String&) {}
void WBTabbedComponent::popupMenuClickOnTab (const int, const String&) {}

void WBTabbedComponent::requestCloseTab(int tabIndex, String name)
{
    std::cout<<"WBTabbedComponent::requestCloseTab "<<(std::string)name.toUTF8()<<" index="<<tabIndex<<std::endl;
    if(wsf_!=0)
    {
        wsf_->removeTab(name);
    }
}


