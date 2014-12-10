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

/*
#include "juce_TabbedComponent.h"
#include "../menus/juce_PopupMenu.h"
#include "../lookandfeel/juce_LookAndFeel.h"
*/

#include "MainComponent.h"
#include "TabbedButtonBar2.h"

//==============================================================================
TabBarButton2::TabBarButton2 (const String& name,
                            TabbedButtonBar2* const owner_,
                            const int index)
    : Button (name),
      owner (owner_),
      tabIndex (index),
      overlapPixels (0),
      closeButton (0),
      showCloseButton (false),
      isAddTabButton (false)
{
    shadow.setShadowProperties (DropShadow(Colour(0.f,0.f,0.f,0.7f), 2, Point<int>(0,0)));
    setComponentEffect (&shadow);
    setWantsKeyboardFocus (false);

	addChildComponent(closeButton = createTabBarCloseButton());
	closeButton->setAlwaysOnTop(true);
	closeButton->setTriggeredOnMouseDown(true);
    closeButton->setTooltip("Close tab");
    
    setShowCloseButton(showCloseButton);
}

TabBarButton2::~TabBarButton2()
{
	delete closeButton;
}

void TabBarButton2::setShowCloseButton(bool show)
{
    showCloseButton = show;
    
    closeButton->setVisible(showCloseButton);

    
}

void TabBarButton2::setIsAddTabButton(bool isAddTabButton_)
{
    isAddTabButton = isAddTabButton_;
}

void TabBarButton2::resized()
{
	float size = (float)getHeight()*0.8f;
	
	closeButton->setSize((int)size,(int)size);
	closeButton->setCentrePosition(getWidth()-(int)(size), (getHeight()/2)+2);
}

int TabBarButton2::getTabButtonBestWidth (int tabDepth)
{
    int width = Font (tabDepth * 0.6f).getStringWidth (getButtonText().trim())
                  + owner->getTabButtonOverlap (tabDepth) * 2;

    return jlimit (tabDepth * 2, tabDepth * 8, width);
}

int TabBarButton2::getBestTabLength (const int depth)
{
    if(isAddTabButton)
    {
        return 44;
    }
    else 
    {
        #if STAGE_BUILD==DESKTOP
        return jlimit (depth * 2,
                       depth * 7,
                       getTabButtonBestWidth (depth))+48;
        #elif STAGE_BUILD==IOS
        return jlimit (depth * 2,
                       depth * 7,
                       getTabButtonBestWidth (depth))+2;
        #endif // STAGE_BUILD
    }
}

Button* TabBarButton2::createTabBarCloseButton()
{
    const float thickness = 7.0f;
    const float indent = 22.0f;
	
    Path p;
    p.addEllipse (-10.0f, -10.0f, 120.0f, 120.0f);
	
    DrawablePath ellipse;
    ellipse.setPath (p);
    ellipse.setFill (Colour (0x99ffffff));
	
    p.clear();
    p.addEllipse (0.0f, 0.0f, 100.0f, 100.0f);
    p.addRectangle (indent, 50.0f - thickness, 100.0f - indent * 2.0f, thickness * 2.0f);
    p.addRectangle (50.0f - thickness, indent, thickness * 2.0f, 50.0f - indent - thickness);
    p.addRectangle (50.0f - thickness, 50.0f + thickness, thickness * 2.0f, 50.0f - indent - thickness);    
	p.setUsingNonZeroWinding (false);	
	p.applyTransform(AffineTransform::rotation(3.141f/4.0f, 50.0f, 50.0f));
    
    DrawablePath dp;
    dp.setPath (p);
    dp.setFill (Colour (0x59000000));
	
	
	
    DrawableComposite normalImage;
    normalImage.addAndMakeVisible (ellipse.createCopy());
    normalImage.addAndMakeVisible (dp.createCopy());
	
    dp.setFill (Colour (0xcc000000));
	
    DrawableComposite overImage;
    overImage.addAndMakeVisible (ellipse.createCopy());
    overImage.addAndMakeVisible (dp.createCopy());
	
    DrawableButton* db = new DrawableButton ("closeTab", DrawableButton::ImageFitted);
    db->setImages (&normalImage, &overImage, 0);
    return db;
}



void TabBarButton2::drawAddTabButton (Graphics& g, int x, int y, int w, int h, bool isMouseOver)
{

    const float thickness = 7.0f;
    const float indent = 22.0f;
	
    Path p;
    p.addEllipse (-10.0f, -10.0f, 120.0f, 120.0f);
	
    DrawablePath ellipse;
    ellipse.setPath (p);
    ellipse.setFill (Colour (0x99ffffff));
	
    p.clear();
    p.addEllipse (0.0f, 0.0f, 100.0f, 100.0f);
    p.addRectangle (indent, 50.0f - thickness, 100.0f - indent * 2.0f, thickness * 2.0f);
    p.addRectangle (50.0f - thickness, indent, thickness * 2.0f, 50.0f - indent - thickness);
    p.addRectangle (50.0f - thickness, 50.0f + thickness, thickness * 2.0f, 50.0f - indent - thickness);    
	p.setUsingNonZeroWinding (false);	
    
    DrawablePath dp;
    dp.setPath (p);
    dp.setFill (Colour (0x59000000));
	
	
	
    DrawableComposite normalImage;
    normalImage.addAndMakeVisible (ellipse.createCopy());
    normalImage.addAndMakeVisible (dp.createCopy());
	
    dp.setFill (Colour (0xcc000000));
	
    DrawableComposite overImage;
    overImage.addAndMakeVisible (ellipse.createCopy());
    overImage.addAndMakeVisible (dp.createCopy());

    float size = (float)h*0.65f;
	
	//g.setColour(Colour(0xffff0000));
    //g.drawRect(x, y+(int)(0.2*(float)h), w, (int)size, 1);
    
    if(isMouseOver)
        overImage.drawWithin(g, juce::Rectangle<float>(x, y+(int)(0.2*(float)h), w, (int)size), RectanglePlacement::centred, 1);
    else 
        normalImage.drawWithin(g, juce::Rectangle<float>(x, y+(int)(0.2*(float)h), w, (int)size), RectanglePlacement::centred, 1);

}


void TabBarButton2::drawTabButton (Graphics& g,
                                      int w, int h,
                                      const Colour& preferredColour,
                                      int tabIndex,
                                      const String& text,
                                      Button& button,
                                      TabbedButtonBar::Orientation orientation,
                                      const bool isMouseOver,
                                      const bool isMouseDown,
                                      const bool isFrontTab)
{
    int length = w;
    int depth = h;
    
    if (orientation == TabbedButtonBar::TabsAtLeft
        || orientation == TabbedButtonBar::TabsAtRight)
    {
        swapVariables (length, depth);
    }
    
    const int indent = owner->getTabButtonOverlap (depth);
    int x = 0, y = 0;
    
    if (orientation == TabbedButtonBar::TabsAtLeft
        || orientation == TabbedButtonBar::TabsAtRight)
    {
        y += indent;
        h -= indent * 2;
    }
    else
    {
        x += indent;
        w -= indent * 2;
    }
    
    Path tabShape;
    createTabButtonShape (tabShape, isMouseOver, isMouseDown);
    
    if(!showCloseButton && !isAddTabButton)
    {
        DropShadow (Colours::black.withAlpha (0.5f), 2, Point<int> (0, 1)).drawForPath (g, tabShape);
        fillTabButtonShape (g, tabShape, preferredColour, isMouseOver, isMouseDown, isFrontTab);
    
        drawTabButtonText (g, x, y, w, h, preferredColour,
                           tabIndex, text, button, orientation,
                           isMouseOver, isMouseDown, isFrontTab);
    }
    else if (isAddTabButton)
    {
        fillTabButtonShape (g, tabShape, preferredColour, isMouseOver, isMouseDown, isFrontTab);
        // draw + for 'add tab' tab
        drawAddTabButton(g, x, y, w, h, isMouseOver);
    }
    else if(showCloseButton)
    {
        fillTabButtonShape (g, tabShape, preferredColour, isMouseOver, isMouseDown, isFrontTab);
        // moved text into centre as there is new space at edges for buttons
        drawTabButtonText (g, x+16, y, w-32, h, preferredColour,
                           tabIndex, text, button, orientation,
                           isMouseOver, isMouseDown, isFrontTab);
    }
}

void TabBarButton2::createTabButtonShape (Path& p, bool /*isMouseOver*/, bool /*isMouseDown*/)
{
    int x, y, w, h;
    getActiveArea (x, y, w, h);

    float length = w;
    float depth = h;

    TabbedButtonBar2::Orientation orientation = owner->getOrientation();
    if (orientation == TabbedButtonBar2::TabsAtLeft
         || orientation == TabbedButtonBar2::TabsAtRight)
    {
        std::swap (length, depth);
    }

    const float indent = (float) owner->getTabButtonOverlap ((int) depth);
    const float overhang = 4.0f;

    switch (orientation)
    {
        case TabbedButtonBar2::TabsAtLeft:
            p.startNewSubPath (w, 0.0f);
            p.lineTo (0.0f, indent);
            p.lineTo (0.0f, h - indent);
            p.lineTo (w, h);
            p.lineTo (w + overhang, h + overhang);
            p.lineTo (w + overhang, -overhang);
            break;

        case TabbedButtonBar2::TabsAtRight:
            p.startNewSubPath (0.0f, 0.0f);
            p.lineTo (w, indent);
            p.lineTo (w, h - indent);
            p.lineTo (0.0f, h);
            p.lineTo (-overhang, h + overhang);
            p.lineTo (-overhang, -overhang);
            break;

        case TabbedButtonBar2::TabsAtBottom:
            p.startNewSubPath (0.0f, 0.0f);
            p.lineTo (indent, h);
            p.lineTo (w - indent, h);
            p.lineTo (w, 0.0f);
            p.lineTo (w + overhang, -overhang);
            p.lineTo (-overhang, -overhang);
            break;

        default:
            p.startNewSubPath (0.0f, h);
            p.lineTo (indent, 0.0f);
            p.lineTo (w - indent, 0.0f);
            p.lineTo (w, h);
            p.lineTo (w + overhang, h + overhang);
            p.lineTo (-overhang, h + overhang);
            break;
    }

    p.closeSubPath();

    p = p.createPathWithRoundedCorners (3.0f);
}

void TabBarButton2::fillTabButtonShape (Graphics& g, const Path& path, const Colour& preferredBackgroundColour, bool /*isMouseOver*/, bool /*isMouseDown*/, const bool isFrontTab)
{
    const Colour tabBackground (preferredBackgroundColour);

    g.setColour (isFrontTab ? tabBackground
                            : tabBackground.withMultipliedAlpha (0.9f));

    g.fillPath (path);

    g.setColour (findColour (isFrontTab ? TabbedButtonBar::frontOutlineColourId
                                               : TabbedButtonBar::tabOutlineColourId, false)
                    .withMultipliedAlpha (isEnabled() ? 1.0f : 0.5f));

    g.strokePath (path, PathStrokeType (isFrontTab ? 1.0f : 0.5f));
}


void TabBarButton2::drawTabButtonText (Graphics& g,
                                          int x, int y, int w, int h,
                                          const Colour& preferredBackgroundColour,
                                          int /*tabIndex*/,
                                          const String& text,
                                          Button& button,
                                          TabbedButtonBar::Orientation orientation,
                                          const bool isMouseOver,
                                          const bool isMouseDown,
                                          const bool isFrontTab)
{
    int length = w;
    int depth = h;
    
	//g.setColour(Colours::steelblue);
	//g.drawRect(x, y, w, h, 1);
	
    if (orientation == TabbedButtonBar::TabsAtLeft
		|| orientation == TabbedButtonBar::TabsAtRight)
    {
        swapVariables (length, depth);
    }
	
    Font font (depth * 0.6f);
    font.setUnderline (button.hasKeyboardFocus (false));
    
	// removed auto-shrinking of text
    GlyphArrangement textLayout;
    textLayout.addFittedText (font, text.trim(),
                              0.0f, 0.0f, (float) length, (float) depth,
                              Justification::centred, 1, 1);
    //                              jmax (1, depth / 12));
	
    AffineTransform transform;
	
    if (orientation == TabbedButtonBar::TabsAtLeft)
    {
        transform = transform.rotated (float_Pi * -0.5f)
		.translated ((float) x, (float) (y + h));
    }
    else if (orientation  == TabbedButtonBar::TabsAtRight)
    {
        transform = transform.rotated (float_Pi * 0.5f)
		.translated ((float) (x + w), (float) y);
    }
    else
    {
        transform = transform.translated ((float) x, (float) y);
    }
	
    if (isFrontTab && (button.isColourSpecified (TabbedButtonBar::frontTextColourId) || isColourSpecified (TabbedButtonBar::frontTextColourId)))
        g.setColour (findColour (TabbedButtonBar::frontTextColourId));
    else if (button.isColourSpecified (TabbedButtonBar::tabTextColourId) || isColourSpecified (TabbedButtonBar::tabTextColourId))
        g.setColour (findColour (TabbedButtonBar::tabTextColourId));
    else
        g.setColour (preferredBackgroundColour.contrasting());
	
    if (! (isMouseOver || isMouseDown))
        g.setOpacity (0.8f);
	
    if (! button.isEnabled())
        g.setOpacity (0.3f);
	
    textLayout.draw (g, transform);
}




void TabBarButton2::paintButton (Graphics& g,
                                bool isMouseOverButton,
                                bool isButtonDown)
{

    int x, y, w, h;
    getActiveArea (x, y, w, h);

    g.setOrigin (x, y);

    drawTabButton (g, w, h,
                   owner->getTabBackgroundColour (tabIndex),
                   tabIndex, getButtonText(), *this,
                   (TabbedButtonBar::Orientation)(owner->getOrientation()),
                   isMouseOverButton, isButtonDown,
                   getToggleState());
}

void TabBarButton2::clicked (const ModifierKeys& mods)
{
    if(!isAddTabButton)
    {
        if (mods.isPopupMenu())
            owner->popupMenuClickOnTab (tabIndex, getButtonText());
        else
            owner->setCurrentTabIndex (tabIndex);
    }
}

bool TabBarButton2::hitTest (int mx, int my)
{
    int x, y, w, h;
    getActiveArea (x, y, w, h);

    if (owner->getOrientation() == TabbedButtonBar2::TabsAtLeft
         || owner->getOrientation() == TabbedButtonBar2::TabsAtRight)
    {
        if (((unsigned int) mx) < (unsigned int) getWidth()
             && my >= y + overlapPixels
             && my < y + h - overlapPixels)
            return true;
    }
    else
    {
        if (mx >= x + overlapPixels && mx < x + w - overlapPixels
             && ((unsigned int) my) < (unsigned int) getHeight())
            return true;
    }

    Path p;
    createTabButtonShape (p, false, false);

    return p.contains ((float) (mx - x),
                       (float) (my - y));
}


void TabBarButton2::getActiveArea (int& x, int& y, int& w, int& h)
{
    x = 0;
    y = 0;
    int r = getWidth();
    int b = getHeight();

    const int spaceAroundImage = getLookAndFeel().getTabButtonSpaceAroundImage();

    if (owner->getOrientation() != TabbedButtonBar2::TabsAtLeft)
        r -= spaceAroundImage;

    if (owner->getOrientation() != TabbedButtonBar2::TabsAtRight)
        x += spaceAroundImage;

    if (owner->getOrientation() != TabbedButtonBar2::TabsAtBottom)
        y += spaceAroundImage;

    if (owner->getOrientation() != TabbedButtonBar2::TabsAtTop)
        b -= spaceAroundImage;

    w = r - x;
    h = b - y;
}



//==============================================================================
class TabAreaBehindFrontButtonComponent2  : public Component
{
public:
    TabAreaBehindFrontButtonComponent2 (TabbedButtonBar2* const owner_)
        : owner (owner_)
    {
        setInterceptsMouseClicks (false, false);
    }

    ~TabAreaBehindFrontButtonComponent2()
    {
    }

    void paint (Graphics& g)
    {
		drawTabAreaBehindFrontButton (g, getWidth(), getHeight(),
									   *owner, owner->getOrientation());
    }

	void drawTabAreaBehindFrontButton (Graphics& g,
										int w, int h,
										TabbedButtonBar2& tabBar,
										TabbedButtonBar2::Orientation orientation)
	{
		const float shadowSize = 0.2f;
		
		float x1 = 0.0f, y1 = 0.0f, x2 = 0.0f, y2 = 0.0f;
        juce::Rectangle<int> shadowRect;
		
		if (orientation == TabbedButtonBar2::TabsAtLeft)
		{
			x1 = (float) w;
			x2 = w * (1.0f - shadowSize);
			shadowRect.setBounds ((int) x2, 0, w - (int) x2, h);
		}
		else if (orientation == TabbedButtonBar2::TabsAtRight)
		{
			x2 = w * shadowSize;
			shadowRect.setBounds (0, 0, (int) x2, h);
		}
		else if (orientation == TabbedButtonBar2::TabsAtBottom)
		{
			y2 = h * shadowSize;
			shadowRect.setBounds (0, 0, w, (int) y2);
		}
		else
		{
			y1 = (float) h;
			y2 = h * (1.0f - shadowSize);
			shadowRect.setBounds (0, (int) y2, w, h - (int) y2);
		}
		
		g.setGradientFill (ColourGradient (Colours::black.withAlpha (tabBar.isEnabled() ? 0.3f : 0.15f), x1, y1,
										   Colours::transparentBlack, x2, y2, false));
		
		shadowRect.expand (2, 2);
		g.fillRect (shadowRect);
		
		g.setColour (Colour (0x80000000));
		
		if (orientation == TabbedButtonBar2::TabsAtLeft)
		{
			g.fillRect (w - 1, 0, 1, h);
		}
		else if (orientation == TabbedButtonBar2::TabsAtRight)
		{
			g.fillRect (0, 0, 1, h);
		}
		else if (orientation == TabbedButtonBar2::TabsAtBottom)
		{
			g.fillRect (0, 0, w, 1);
		}
		else
		{
			g.fillRect (0, h - 1, w, 1);
		}
	}
	
	
    void enablementChanged()
    {
        repaint();
    }

private:
    TabbedButtonBar2* const owner;

    TabAreaBehindFrontButtonComponent2 (const TabAreaBehindFrontButtonComponent2&);
    TabAreaBehindFrontButtonComponent2& operator= (const TabAreaBehindFrontButtonComponent2&);
};


//==============================================================================
TabbedButtonBar2::TabbedButtonBar2 (const Orientation orientation_)
    : orientation (orientation_),
      currentTabIndex (-1),
      extraTabsButton (0),
      addTabTab (0),
      showAddButton (false),
      showCloseButtons (false),
      hideButtons (true),
      showAllTabs (false),
      margin(0)
{
    setInterceptsMouseClicks (false, true);
    addAndMakeVisible (behindFrontTab = new TabAreaBehindFrontButtonComponent2 (this));
    setFocusContainer (true);

    // special tab for adding another tab
    addChildComponent(addTabTab = new TabBarButton2 ("", this, -1));
    addTabTab->setIsAddTabButton(true);
    addTabTab->setShowCloseButton(false);
    addTabTab->setTooltip("Add new tab");
    
    setShowAddButton(showAddButton);
    setShowCloseButtons(showCloseButtons);
}

TabbedButtonBar2::~TabbedButtonBar2()
{
    deleteAllChildren();
}

//==============================================================================
void TabbedButtonBar2::setShowAddButton(bool show)
{
    // set whether the add tab button should be shown
    showAddButton = show;

    addTabTab->setVisible(show);

    resized();
    repaint();
}

void TabbedButtonBar2::setShowCloseButtons(bool show)
{
    // set whether the close tab button should be shown
    showCloseButtons = show;
    
//    for (int i = 0; i < getNumTabs(); i++)
//        getTabButton(i)->setShowCloseButton(show);
//    
//    resized();
//    repaint();
}

void TabbedButtonBar2::hideAddCloseButtons(bool hide)
{
    hideButtons = hide;
    
    // temporarily hide the add and close buttons, without changing their normal viewing state
    if(hideButtons)
    {
        addTabTab->setVisible(false);
        for (int i = 0; i < getNumTabs(); i++)
            getTabButton(i)->setShowCloseButton(false);
    }
    else 
    {
        // restore their set state
        addTabTab->setVisible(showAddButton);
        for (int i = 0; i < getNumTabs(); i++)
            getTabButton(i)->setShowCloseButton(showCloseButtons);
    }
    resized();
    repaint();

}

void TabbedButtonBar2::setOrientation (const Orientation newOrientation)
{
    orientation = newOrientation;

    for (int i = getNumChildComponents(); --i >= 0;)
        getChildComponent (i)->resized();

    resized();
}

TabBarButton2* TabbedButtonBar2::createTabButton (const String& name, const int index)
{
    return new TabBarButton2 (name, this, index);
}


void TabbedButtonBar2::setTabVisible(int tabIndex, bool isVisible)
{
    tabIsVisible.set(tabIndex, isVisible);

    resized();
}

bool TabbedButtonBar2::getTabVisible(int tabIndex)
{
	if (tabIndex < 0)
	{
		return false;
	}
    return tabIsVisible[tabIndex];
}

void TabbedButtonBar2::setShowAllTabs(bool show)
{
    showAllTabs = show;
    resized();
}

//==============================================================================
void TabbedButtonBar2::clearTabs()
{
    tabs.clear();
    tabColours.clear();
    currentTabIndex = -1;

    deleteAndZero (extraTabsButton);
    removeChildComponent (behindFrontTab);
    deleteAllChildren();
    addChildComponent (behindFrontTab);

    setCurrentTabIndex (-1);
}

void TabbedButtonBar2::addTab (const String& tabName,
                              const Colour& tabBackgroundColour,
                              int insertIndex)
{
    jassert (tabName.isNotEmpty()); // you have to give them all a name..

    if (tabName.isNotEmpty())
    {
        if (((unsigned int) insertIndex) > (unsigned int) tabs.size())
            insertIndex = tabs.size();

        for (int i = tabs.size(); --i >= insertIndex;)
        {
            TabBarButton2* const tb = getTabButton (i);

            if (tb != 0)
                tb->tabIndex++;
        }

        tabs.insert (insertIndex, tabName);
        tabColours.insert (insertIndex, tabBackgroundColour);
        tabIsVisible.insert (insertIndex, true);

        TabBarButton2* const tb = createTabButton (tabName, insertIndex);
        jassert (tb != 0); // your createTabButton() mustn't return zero!

        addAndMakeVisible (tb, insertIndex);

        tb->setShowCloseButton(showCloseButtons && !hideButtons);
        
        resized();

        if (currentTabIndex < 0)
            setCurrentTabIndex (0);
    }
}

void TabbedButtonBar2::setTabName (const int tabIndex,
                                  const String& newName)
{
    if (((unsigned int) tabIndex) < (unsigned int) tabs.size()
         && tabs[tabIndex] != newName)
    {
        tabs.set (tabIndex, newName);

        TabBarButton2* const tb = getTabButton (tabIndex);

        if (tb != 0)
            tb->setButtonText (newName);

        resized();
    }
}

void TabbedButtonBar2::removeTab (const int tabIndex)
{
    if (((unsigned int) tabIndex) < (unsigned int) tabs.size())
    {
        const int oldTabIndex = currentTabIndex;
        if (currentTabIndex == tabIndex)
            currentTabIndex = -1;

        tabs.remove (tabIndex);
        tabColours.remove (tabIndex);

        delete getTabButton (tabIndex);

        for (int i = tabIndex + 1; i <= tabs.size(); ++i)
        {
            TabBarButton2* const tb = getTabButton (i);

            if (tb != 0)
                tb->tabIndex--;
        }

        resized();

        setCurrentTabIndex (jlimit (0, jmax (0, tabs.size() - 1), oldTabIndex));
    }
}

void TabbedButtonBar2::moveTab (const int currentIndex,
                               const int newIndex)
{
    tabs.move (currentIndex, newIndex);
    tabColours.move (currentIndex, newIndex);
    resized();
}

int TabbedButtonBar2::getNumTabs() const
{
    return tabs.size();
}

const StringArray TabbedButtonBar2::getTabNames() const
{
    return tabs;
}

void TabbedButtonBar2::setCurrentTabIndex (int newIndex, const bool sendChangeMessage_)
{
    if (currentTabIndex != newIndex)
    {
        if (((unsigned int) newIndex) >= (unsigned int) tabs.size())
            newIndex = -1;

        currentTabIndex = newIndex;

        for (int i = 0; i < getNumChildComponents(); ++i)
        {
            TabBarButton2* const tb = dynamic_cast <TabBarButton2*> (getChildComponent (i));

            if (tb != 0)
                tb->setToggleState (tb->tabIndex == newIndex, dontSendNotification);
        }

        resized();

        if (sendChangeMessage_)
            sendChangeMessage ();
		
        currentTabChanged (newIndex, newIndex >= 0 ? tabs [newIndex] : String::empty);
    }
}

TabBarButton2* TabbedButtonBar2::getTabButton (const int index) const
{
    for (int i = getNumChildComponents(); --i >= 0;)
    {
        TabBarButton2* const tb = dynamic_cast <TabBarButton2*> (getChildComponent (i));

        if (tb != 0 && tb->tabIndex == index)
            return tb;
    }

    return 0;
}

void TabbedButtonBar2::lookAndFeelChanged()
{
    deleteAndZero (extraTabsButton);
    resized();
}

int TabbedButtonBar2::getTabButtonOverlap (int tabDepth)
{
    return 1 + tabDepth / 3;
}

void TabbedButtonBar2::resized()
{
//    const double minimumScale = 0.7;
    const double minimumScale = 1.0;
    int depth = getWidth();
    int length = getHeight();

    if (orientation == TabsAtTop || orientation == TabsAtBottom)
        swapVariables (depth, length);

    const int overlap = getTabButtonOverlap (depth)
                            + getLookAndFeel().getTabButtonSpaceAroundImage() * 2;

    int i, totalLength = overlap + addTabTab->getBestTabLength(depth);
    int numVisibleButtons = tabs.size();

    for (i = 0; i < getNumChildComponents(); ++i)
    {
        TabBarButton2* const tb = dynamic_cast <TabBarButton2*> (getChildComponent (i));

        if (tb != 0 && (getTabVisible(tb->tabIndex) || showAllTabs))
        {
            totalLength += tb->getBestTabLength (depth) - overlap;
            tb->overlapPixels = overlap / 2;
        }
    }

    double scale = 1.0;

//    if (totalLength > length)
//        scale = jmax (minimumScale, length / (double) totalLength);

    const bool isTooBig = totalLength * scale > (length-margin);
    int tabsButtonPos = 0;
    if (isTooBig)
    {
        if (extraTabsButton == 0)
        {
            addAndMakeVisible (extraTabsButton = getLookAndFeel().createTabBarExtrasButton());
            extraTabsButton->addListener (this);
            extraTabsButton->setAlwaysOnTop (true);
            extraTabsButton->setTriggeredOnMouseDown (true);
        }

        const int buttonSize = jmin (proportionOfWidth (0.7f), proportionOfHeight (0.7f));
        extraTabsButton->setSize (buttonSize, buttonSize);

        if (orientation == TabsAtTop || orientation == TabsAtBottom)
        {
            tabsButtonPos = (getWidth()-margin) - buttonSize / 2 - 1;
            extraTabsButton->setCentrePosition (tabsButtonPos, getHeight() / 2);
        }
        else
        {
            tabsButtonPos = getHeight() - buttonSize / 2 - 1;
            extraTabsButton->setCentrePosition (getWidth() / 2, tabsButtonPos);
        }

        totalLength = addTabTab->getBestTabLength(depth);

        for (i = 0; i < tabs.size(); ++i)
        {
            TabBarButton2* const tb = getTabButton (i);

            if (tb != 0 && (getTabVisible(tb->tabIndex) || showAllTabs))
            {
                const int newLength = totalLength + tb->getBestTabLength (depth);

                if (i > 0 && newLength * minimumScale > tabsButtonPos)
                {
                    totalLength += overlap;
                    break;
                }

                numVisibleButtons = i + 1;
                totalLength = newLength - overlap;

            }
        }

//        scale = jmax (minimumScale, tabsButtonPos / (double) totalLength);
		scale = 1.0;
    }
    else
    {
        deleteAndZero (extraTabsButton);
    }


//	const int addButtonSize = jmin (proportionOfWidth (0.8f), proportionOfHeight (0.8f));
//	addTabButton->setSize (addButtonSize, addButtonSize);
//	
//	int addButtonPos = getHeight() - addButtonSize / 2 - 2;
//	addTabButton->setCentrePosition (totalLength+4, addButtonPos);
	
    int pos = 0;

    TabBarButton2* frontTab = 0;

    for (i = 0; i < tabs.size(); ++i)
    {
        TabBarButton2* const tb = getTabButton (i);

        if (tb != 0)
        {
            const int bestLength = roundToInt (scale * tb->getBestTabLength (depth));

            if (i < numVisibleButtons && (getTabVisible(tb->tabIndex) || showAllTabs))
            {
                if (orientation == TabsAtTop || orientation == TabsAtBottom)
                    tb->setBounds (pos, 0, bestLength, getHeight());
                else
                    tb->setBounds (0, pos, getWidth(), bestLength);

                tb->toBack();

                if (tb->tabIndex == currentTabIndex)
                    frontTab = tb;

                tb->setVisible (true);

                pos += bestLength - overlap;

            }
            else
            {
                tb->setVisible (false);
            }

        }
    }

    TabBarButton2* const atb = getTabButton(-1);
    const int bestLength = roundToInt (scale * atb->getBestTabLength (depth));
    if (orientation == TabsAtTop || orientation == TabsAtBottom)
        atb->setBounds (pos, 0, bestLength, getHeight());
    else
        atb->setBounds (0, totalLength, getWidth(), bestLength);
//    atb->setVisible(true);
    atb->toBack();

    behindFrontTab->setBounds (0, 0, getWidth(), getHeight());

    if (frontTab != 0)
    {
        frontTab->toFront (false);
        behindFrontTab->toBehind (frontTab);
    }
}

//==============================================================================
const Colour TabbedButtonBar2::getTabBackgroundColour (const int tabIndex)
{
    if(tabIndex==-1)
        return Colour(0xfff7f7f7);
    else
        return tabColours [tabIndex];
}

void TabbedButtonBar2::setTabBackgroundColour (const int tabIndex, const Colour& newColour)
{
    if (((unsigned int) tabIndex) < (unsigned int) tabColours.size()
         && tabColours [tabIndex] != newColour)
    {
        tabColours.set (tabIndex, newColour);
        repaint();
    }
}

void TabbedButtonBar2::buttonClicked (Button* button)
{
    if (extraTabsButton == button)
    {
        PopupMenu m;

        for (int i = 0; i < tabs.size(); ++i)
        {
            TabBarButton2* const tb = getTabButton (i);

            if (tb != 0 && ! tb->isVisible() && getTabVisible(tb->tabIndex))
			{
                m.addItem (tb->tabIndex + 1, tabs[i], true, i == currentTabIndex);
			}
        }

        const int res = m.showAt (extraTabsButton);

        if (res != 0)
		{
            setCurrentTabIndex (res - 1);
		}
    }

}

//==============================================================================
void TabbedButtonBar2::currentTabChanged (const int, const String&)
{
}

void TabbedButtonBar2::popupMenuClickOnTab (const int, const String&)
{
}

//END_JUCE_NAMESPACE
