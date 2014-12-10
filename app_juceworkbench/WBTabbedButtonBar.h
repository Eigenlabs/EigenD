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

#ifndef __WBTABBEDBUTTONBAR__
#define __WBTABBEDBUTTONBAR__
#include "juce.h"

class WBTabbedButtonBar;

/** In a TabbedButtonBar, this component is used for each of the buttons.

	If you want to create a TabbedButtonBar with custom tab components, derive
	your component from this class, and override the TabbedButtonBar::createTabButton()
	method to create it instead of the default one.

	@see TabbedButtonBar
*/
class WBTabBarButton  : public Button,
                        public Button::Listener
{
public:

	/** Creates the tab button. */
	WBTabBarButton (const String& name, WBTabbedButtonBar& ownerBar);

	/** Destructor. */
	~WBTabBarButton();

	/** Chooses the best length for the tab, given the specified depth.

		If the tab is horizontal, this should return its width, and the depth
		specifies its height. If it's vertical, it should return the height, and
		the depth is actually its width.
	*/
	virtual int getBestTabLength (int depth);
	int getTabButtonBestWidth (int depth);

	void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown);
    void drawTabButtonText (Graphics& g,
									int x, int y, int w, int h,
									const Colour& preferredBackgroundColour,
									int tabIndex,
									const String& text,
									Button& button,
									TabbedButtonBar::Orientation orientation,
									bool isMouseOver,
									bool isMouseDown,
									bool isFrontTab);


	void drawTabButton (Graphics& g,
								int w, int h,
								const Colour& preferredColour,
								int tabIndex,
								const String& text,
								Button& button,
								TabbedButtonBar::Orientation orientation,
								bool isMouseOver,
								bool isMouseDown,
								bool isFrontTab);

	void clicked (const ModifierKeys& mods);
	bool hitTest (int x, int y);

    void setShowCloseButton(bool show);
    void resized();
    Button* createTabBarCloseButton();
    virtual void buttonClicked(Button*);

//    int getTabButtonOverlap (int tabDepth);

protected:

	friend class WBTabbedButtonBar;
	WBTabbedButtonBar& owner;
	int overlapPixels;
	DropShadowEffect shadow;

	/** Returns an area of the component that's safe to draw in.

		This deals with the orientation of the tabs, which affects which side is
		touching the tabbed box's content component.
	*/
	void getActiveArea(int& x, int& y, int& w, int& h);
    void createTabButtonShape (Path& p, bool /*isMouseOver*/, bool /*isMouseDown*/);
    void fillTabButtonShape (Graphics& g, const Path& path, const Colour& preferredBackgroundColour,  bool /*isMouseOver*/, bool /*isMouseDown*/, const bool isFrontTab);
 
	/** Returns this tab's index in its tab bar. */
	int getIndex() const;
  	Button* closeButton;
    bool showCloseButton;

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WBTabBarButton);
};

/**
	A vertical or horizontal bar containing tabs that you can select.

	You can use one of these to generate things like a dialog box that has
	tabbed pages you can flip between. Attach a ChangeListener to the
	button bar to be told when the user changes the page.

	An easier method than doing this is to use a TabbedComponent, which
	contains its own TabbedButtonBar and which takes care of the layout
	and other housekeeping.

	@see TabbedComponent
*/
class  WBTabbedButtonBar  : public Component,
								   public ChangeBroadcaster
{
public:

	/** The placement of the tab-bar

		@see setOrientation, getOrientation
	*/
	enum Orientation
	{
		TabsAtTop,
		TabsAtBottom,
		TabsAtLeft,
		TabsAtRight
	};

	/** Creates a TabbedButtonBar with a given placement.

		You can change the orientation later if you need to.
	*/
	WBTabbedButtonBar (Orientation orientation);

	/** Destructor. */
	~WBTabbedButtonBar();

	/** Changes the bar's orientation.

		This won't change the bar's actual size - you'll need to do that yourself,
		but this determines which direction the tabs go in, and which side they're
		stuck to.
	*/
	void setOrientation (Orientation orientation);

	/** Returns the current orientation.

		@see setOrientation
	*/
	Orientation getOrientation() const noexcept			 { return orientation; }

	/** Changes the minimum scale factor to which the tabs can be compressed when trying to
		fit a lot of tabs on-screen.
	*/
	void setMinimumTabScaleFactor (double newMinimumScale);

	/** Deletes all the tabs from the bar.

		@see addTab
	*/
	void clearTabs();

	/** Adds a tab to the bar.

		Tabs are added in left-to-right reading order.

		If this is the first tab added, it'll also be automatically selected.
	*/
	void addTab (const String& tabName,
				 const Colour& tabBackgroundColour,
				 int insertIndex = -1);

	/** Changes the name of one of the tabs. */
	void setTabName (int tabIndex,
					 const String& newName);

	/** Gets rid of one of the tabs. */
	void removeTab (int tabIndex);

	/** Moves a tab to a new index in the list.

		Pass -1 as the index to move it to the end of the list.
	*/
	void moveTab (int currentIndex, int newIndex);

	/** Returns the number of tabs in the bar. */
	int getNumTabs() const;

	/** Returns a list of all the tab names in the bar. */
	StringArray getTabNames() const;

	/** Changes the currently selected tab.

		This will send a change message and cause a synchronous callback to
		the currentTabChanged() method. (But if the given tab is already selected,
		nothing will be done).

		To deselect all the tabs, use an index of -1.
	*/
	void setCurrentTabIndex (int newTabIndex, bool sendChangeMessage = true);
    void setCloseButtonClicked (int index, String name);

	/** Returns the name of the currently selected tab.

		This could be an empty string if none are selected.
	*/
	String getCurrentTabName() const;

	/** Returns the index of the currently selected tab.

		This could return -1 if none are selected.
	*/
	int getCurrentTabIndex() const noexcept				 { return currentTabIndex; }

	/** Returns the button for a specific tab.

		The button that is returned may be deleted later by this component, so don't hang
		on to the pointer that is returned. A null pointer may be returned if the index is
		out of range.
	*/
	WBTabBarButton* getTabButton (int index) const;

	/** Returns the index of a TabBarButton if it belongs to this bar. */
	int indexOfTabButton (const WBTabBarButton* button) const;

	/** Callback method to indicate the selected tab has been changed.

		@see setCurrentTabIndex
	*/
	virtual void currentTabChanged (int newCurrentTabIndex,
									const String& newCurrentTabName);

    virtual void closeButtonClicked(int index, String name){};

	/** Callback method to indicate that the user has right-clicked on a tab.

		(Or ctrl-clicked on the Mac)
	*/
	virtual void popupMenuClickOnTab (int tabIndex, const String& tabName);

	/** Returns the colour of a tab.

		This is the colour that was specified in addTab().
	*/
	const Colour getTabBackgroundColour (int tabIndex);

	/** Changes the background colour of a tab.

		@see addTab, getTabBackgroundColour
	*/
	void setTabBackgroundColour (int tabIndex, const Colour& newColour);

	/** A set of colour IDs to use to change the colour of various aspects of the component.

		These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
		methods.

		@see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
	*/
	enum ColourIds
	{
		tabOutlineColourId		  = 0x1005812,	/**< The colour to use to draw an outline around the tabs.  */
		tabTextColourId		 = 0x1005813,	/**< The colour to use to draw the tab names. If this isn't specified,
															 the look and feel will choose an appropriate colour. */
		frontOutlineColourId		= 0x1005814,	/**< The colour to use to draw an outline around the currently-selected tab.  */
		frontTextColourId		   = 0x1005815,	/**< The colour to use to draw the currently-selected tab name. If
															 this isn't specified, the look and feel will choose an appropriate
															 colour. */
	};

	/** @internal */
	void resized();
	/** @internal */
	void lookAndFeelChanged();

    int getTabButtonOverlap (int tabDepth);
protected:

	/** This creates one of the tabs.

		If you need to use custom tab components, you can override this method and
		return your own class instead of the default.
	*/
	virtual WBTabBarButton* createTabButton (const String& tabName, int tabIndex);

private:
	Orientation orientation;

	struct TabInfo
	{
		ScopedPointer<WBTabBarButton> component;
		String name;
		Colour colour;
	};

	OwnedArray <TabInfo> tabs;

	double minimumScale;
	int currentTabIndex;

	class BehindFrontTabComp;
	friend class BehindFrontTabComp;
	friend class ScopedPointer<BehindFrontTabComp>;
	ScopedPointer<BehindFrontTabComp> behindFrontTab;
	ScopedPointer<Button> extraTabsButton;

	void showExtraItemsMenu();
	static void extraItemsMenuCallback (int, WBTabbedButtonBar*);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WBTabbedButtonBar);
};

#endif 


