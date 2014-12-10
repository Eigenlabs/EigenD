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

#include "WBTabbedButtonBar.h"

WBTabBarButton::WBTabBarButton (const String& name, WBTabbedButtonBar& owner_)
	: Button (name),
	  owner (owner_),
	  overlapPixels (0),
      closeButton (0),
      showCloseButton (!(name.equalsIgnoreCase("main workspace")))
{
	shadow.setShadowProperties(DropShadow(Colour(0.f,0.f,0.f,0.7f),2, Point<int>(0,0)));
	setComponentEffect (&shadow);
	setWantsKeyboardFocus (false);

	addChildComponent(closeButton = createTabBarCloseButton());
	closeButton->setAlwaysOnTop(true);
	closeButton->setTriggeredOnMouseDown(true);
    closeButton->setTooltip("Close tab");
    closeButton->addListener(this);
    
    setShowCloseButton(showCloseButton);

}

WBTabBarButton::~WBTabBarButton()
{
	delete closeButton;
}

void WBTabBarButton::buttonClicked(Button* b)
{
    std::cout<<"WBTabBarButton::buttonClicked"<<std::endl;    
    owner.setCloseButtonClicked(getIndex(), getButtonText());
}

void WBTabBarButton::setShowCloseButton(bool show)
{
    showCloseButton = show;
    
    closeButton->setVisible(showCloseButton);
}

Button* WBTabBarButton::createTabBarCloseButton()
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


    //normalImage.insertDrawable (ellipse);
    //normalImage.insertDrawable (dp);
	
    dp.setFill (Colour (0xcc000000));
	
    DrawableComposite overImage;
    //overImage.insertDrawable (ellipse);
    //overImage.insertDrawable (dp);

	overImage.addAndMakeVisible (ellipse.createCopy());
	overImage.addAndMakeVisible (dp.createCopy());

    DrawableButton* db = new DrawableButton ("closeTab", DrawableButton::ImageFitted);
    db->setImages (&normalImage, &overImage, 0);
    return db;
}

void WBTabBarButton::resized()
{
	float size = (float)getHeight()*0.8f;
	
	closeButton->setSize((int)size,(int)size);
	closeButton->setCentrePosition(getWidth()-(int)(size), (getHeight()/2)+2);
}

int WBTabBarButton::getIndex() const
{
	return owner.indexOfTabButton (this);
}

void WBTabBarButton::paintButton (Graphics& g,
								bool isMouseOverButton,
								bool isButtonDown)
{
    int x, y, w, h;
    getActiveArea (x, y, w, h);

    g.setOrigin (x, y);



//    if(!showCloseButton)
//    {
//	getLookAndFeel()
//		.drawTabButton (g, area.getWidth(), area.getHeight(),
//						owner.getTabBackgroundColour (getIndex()),
//						getIndex(), getButtonText(), *this,
//						(TabbedButtonBar::Orientation)owner.getOrientation(),
//						isMouseOverButton, isButtonDown,
//						getToggleState());
//    }
//    else
//    {
		drawTabButton (g, w, h,
						owner.getTabBackgroundColour (getIndex()),
						getIndex(), getButtonText(), *this,
						(TabbedButtonBar::Orientation)owner.getOrientation(),
						isMouseOverButton, isButtonDown,
						getToggleState());
//    }

}
void WBTabBarButton::drawTabButton (Graphics& g,
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
		std::swap (length, depth);
	}

	Path tabShape;

	createTabButtonShape (tabShape, isMouseOver, isMouseDown);
	fillTabButtonShape (g, tabShape, preferredColour,
						isMouseOver, isMouseDown, isFrontTab);

	const int indent = owner.getTabButtonOverlap (depth);
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

	drawTabButtonText (g, x, y, w-16, h, preferredColour,
					   tabIndex, text, button, orientation,
					   isMouseOver, isMouseDown, isFrontTab);
}

void WBTabBarButton::drawTabButtonText (Graphics& g,
									 int x, int y, int w, int h,
									 const Colour& preferredBackgroundColour,
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
		std::swap (length, depth);
	}

	Font font (depth * 0.6f);
	font.setUnderline (button.hasKeyboardFocus (false));

	GlyphArrangement textLayout;
	textLayout.addFittedText (font, text.trim(),
							  0.0f, 0.0f, (float) length, (float) depth,
							  Justification::centred,1,1);
//							  jmax (1, depth / 12));

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



void WBTabBarButton::clicked (const ModifierKeys& mods)
{
	if (mods.isPopupMenu())
		owner.popupMenuClickOnTab (getIndex(), getButtonText());
	else
		owner.setCurrentTabIndex (getIndex());
}

bool WBTabBarButton::hitTest (int mx, int my)
{
    int x, y, w, h;
    getActiveArea (x, y, w, h);

    if (owner.getOrientation() == WBTabbedButtonBar::TabsAtLeft
         || owner.getOrientation() == WBTabbedButtonBar::TabsAtRight)
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

int WBTabBarButton::getTabButtonBestWidth (int tabDepth)
{
    int width = Font (tabDepth * 0.6f).getStringWidth ((getButtonText()+"XX").trim())
                  + owner.getTabButtonOverlap (tabDepth) * 2;

    return jlimit (tabDepth * 2, tabDepth * 8, width);
}

int WBTabBarButton::getBestTabLength (const int depth)
{
	return jlimit (depth * 2,
				   depth * 7,
				   getTabButtonBestWidth (depth));
}

//const juce::Rectangle<int> WBTabBarButton::getActiveArea()
//{
//	juce::Rectangle<int> r (getLocalBounds());
//	const int spaceAroundImage = getLookAndFeel().getTabButtonSpaceAroundImage();
//
//	if (owner.getOrientation() != WBTabbedButtonBar::TabsAtLeft)	  r.removeFromRight (spaceAroundImage);
//	if (owner.getOrientation() != WBTabbedButtonBar::TabsAtRight)	 r.removeFromLeft (spaceAroundImage);
//	if (owner.getOrientation() != WBTabbedButtonBar::TabsAtBottom)	r.removeFromTop (spaceAroundImage);
//	if (owner.getOrientation() != WBTabbedButtonBar::TabsAtTop)	   r.removeFromBottom (spaceAroundImage);
//
//	return r;
//}
void WBTabBarButton::getActiveArea (int& x, int& y, int& w, int& h)
{
    x = 0;
    y = 0;
    int r = getWidth();
    int b = getHeight();

    const int spaceAroundImage = getLookAndFeel().getTabButtonSpaceAroundImage();

    if (owner.getOrientation() != WBTabbedButtonBar::TabsAtLeft)
        r -= spaceAroundImage;

    if (owner.getOrientation() != WBTabbedButtonBar::TabsAtRight)
        x += spaceAroundImage;

    if (owner.getOrientation() != WBTabbedButtonBar::TabsAtBottom)
        y += spaceAroundImage;

    if (owner.getOrientation() != WBTabbedButtonBar::TabsAtTop)
        b -= spaceAroundImage;

    w = r - x;
    h = b - y;
}


void WBTabBarButton::createTabButtonShape (Path& p, bool /*isMouseOver*/, bool /*isMouseDown*/)
{
    int x, y, w, h;
    getActiveArea (x, y, w, h);

    float length = w;
    float depth = h;

    WBTabbedButtonBar::Orientation orientation = owner.getOrientation();
    if (orientation == WBTabbedButtonBar::TabsAtLeft
         || orientation == WBTabbedButtonBar::TabsAtRight)
    {
        std::swap (length, depth);
    }

    const float indent = (float) owner.getTabButtonOverlap ((int) depth);
    const float overhang = 4.0f;

    switch (orientation)
    {
        case WBTabbedButtonBar::TabsAtLeft:
            p.startNewSubPath (w, 0.0f);
            p.lineTo (0.0f, indent);
            p.lineTo (0.0f, h - indent);
            p.lineTo (w, h);
            p.lineTo (w + overhang, h + overhang);
            p.lineTo (w + overhang, -overhang);
            break;

        case WBTabbedButtonBar::TabsAtRight:
            p.startNewSubPath (0.0f, 0.0f);
            p.lineTo (w, indent);
            p.lineTo (w, h - indent);
            p.lineTo (0.0f, h);
            p.lineTo (-overhang, h + overhang);
            p.lineTo (-overhang, -overhang);
            break;

        case WBTabbedButtonBar::TabsAtBottom:
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

void WBTabBarButton::fillTabButtonShape (Graphics& g, const Path& path, const Colour& preferredBackgroundColour, bool /*isMouseOver*/, bool /*isMouseDown*/, const bool isFrontTab)
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



class WBTabbedButtonBar::BehindFrontTabComp  : public Component,
											 public ButtonListener // (can't use Button::Listener due to idiotic VC2005 bug)
{
public:
	BehindFrontTabComp (WBTabbedButtonBar& owner_)
		: owner (owner_)
	{
		setInterceptsMouseClicks (false, false);
	}

	void paint (Graphics& g)
	{
		//getLookAndFeel().drawTabAreaBehindFrontButton (g, getWidth(), getHeight(),
		drawTabAreaBehindFrontButton (g, getWidth(), getHeight(),
													   owner, owner.getOrientation());
	}

	void drawTabAreaBehindFrontButton (Graphics& g,
										int w, int h,
										WBTabbedButtonBar& tabBar,
										WBTabbedButtonBar::Orientation orientation)
	{
		const float shadowSize = 0.2f;
		
		float x1 = 0.0f, y1 = 0.0f, x2 = 0.0f, y2 = 0.0f;
        juce::Rectangle<int> shadowRect;
		
		if (orientation == WBTabbedButtonBar::TabsAtLeft)
		{
			x1 = (float) w;
			x2 = w * (1.0f - shadowSize);
			shadowRect.setBounds ((int) x2, 0, w - (int) x2, h);
		}
		else if (orientation == WBTabbedButtonBar::TabsAtRight)
		{
			x2 = w * shadowSize;
			shadowRect.setBounds (0, 0, (int) x2, h);
		}
		else if (orientation == WBTabbedButtonBar::TabsAtBottom)
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
		
		g.setGradientFill (ColourGradient (Colours::black.withAlpha (owner.isEnabled() ? 0.3f : 0.15f), x1, y1,
										   Colours::transparentBlack, x2, y2, false));
		
		shadowRect.expand (2, 2);
		g.fillRect (shadowRect);
		
		g.setColour (Colour (0x80000000));
		
		if (orientation == WBTabbedButtonBar::TabsAtLeft)
		{
			g.fillRect (w - 1, 0, 1, h);
		}
		else if (orientation == WBTabbedButtonBar::TabsAtRight)
		{
			g.fillRect (0, 0, 1, h);
		}
		else if (orientation == WBTabbedButtonBar::TabsAtBottom)
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

	void buttonClicked (Button*)
	{
		owner.showExtraItemsMenu();
	}

private:
	WBTabbedButtonBar& owner;

	JUCE_DECLARE_NON_COPYABLE (BehindFrontTabComp);
};

WBTabbedButtonBar::WBTabbedButtonBar (const Orientation orientation_)
	: orientation (orientation_),
	  minimumScale (0.7),
	  currentTabIndex (-1)
{
	setInterceptsMouseClicks (false, true);
	addAndMakeVisible (behindFrontTab = new BehindFrontTabComp (*this));
	setFocusContainer (true);
}

WBTabbedButtonBar::~WBTabbedButtonBar()
{
	tabs.clear();
	extraTabsButton = nullptr;
}

void WBTabbedButtonBar::setOrientation (const Orientation newOrientation)
{
	orientation = newOrientation;

	for (int i = getNumChildComponents(); --i >= 0;)
		getChildComponent (i)->resized();

	resized();
}

WBTabBarButton* WBTabbedButtonBar::createTabButton (const String& name, const int /*index*/)
{
	return new WBTabBarButton (name, *this);
}

void WBTabbedButtonBar::setMinimumTabScaleFactor (double newMinimumScale)
{
	minimumScale = newMinimumScale;
	resized();
}

void WBTabbedButtonBar::clearTabs()
{
	tabs.clear();
	extraTabsButton = nullptr;
	setCurrentTabIndex (-1);
}

void WBTabbedButtonBar::addTab (const String& tabName,
							  const Colour& tabBackgroundColour,
							  int insertIndex)
{
	jassert (tabName.isNotEmpty()); // you have to give them all a name..

	if (tabName.isNotEmpty())
	{
		if (! isPositiveAndBelow (insertIndex, tabs.size()))
			insertIndex = tabs.size();

		TabInfo* newTab = new TabInfo();
		newTab->name = tabName;
		newTab->colour = tabBackgroundColour;
		newTab->component = createTabButton (tabName, insertIndex);

		jassert (newTab->component != nullptr);

		tabs.insert (insertIndex, newTab);
		addAndMakeVisible (newTab->component, insertIndex);

		resized();

		if (currentTabIndex < 0)
			setCurrentTabIndex (0);
	}
}

void WBTabbedButtonBar::setTabName (const int tabIndex, const String& newName)
{
	TabInfo* const tab = tabs [tabIndex];

	if (tab != nullptr && tab->name != newName)
	{
		tab->name = newName;
		tab->component->setButtonText (newName);
		resized();
	}
}

void WBTabbedButtonBar::removeTab (const int tabIndex)
{
	if (tabs [tabIndex] != nullptr)
	{
		const int oldTabIndex = currentTabIndex;
		if (currentTabIndex == tabIndex)
			currentTabIndex = -1;

		tabs.remove (tabIndex);
		resized();

		setCurrentTabIndex (jlimit (0, jmax (0, tabs.size() - 1), oldTabIndex));
	}
}

void WBTabbedButtonBar::moveTab (const int currentIndex, const int newIndex)
{
	tabs.move (currentIndex, newIndex);
	resized();
}

int WBTabbedButtonBar::getNumTabs() const
{
	return tabs.size();
}

String WBTabbedButtonBar::getCurrentTabName() const
{
	TabInfo* tab = tabs [currentTabIndex];
	return tab == nullptr ? String::empty : tab->name;
}

StringArray WBTabbedButtonBar::getTabNames() const
{
	StringArray names;

	for (int i = 0; i < tabs.size(); ++i)
		names.add (tabs.getUnchecked(i)->name);

	return names;
}

void WBTabbedButtonBar::setCloseButtonClicked(int index, String tabName)
{
    closeButtonClicked(index,tabName);
}

void WBTabbedButtonBar::setCurrentTabIndex (int newIndex, const bool sendChangeMessage_)
{
	if (currentTabIndex != newIndex)
	{
		if (! isPositiveAndBelow (newIndex, tabs.size()))
			newIndex = -1;

		currentTabIndex = newIndex;

		for (int i = 0; i < tabs.size(); ++i)
		{
			WBTabBarButton* tb = tabs.getUnchecked(i)->component;
			tb->setToggleState (i == newIndex, false);
		}

		resized();

		if (sendChangeMessage_)
			sendChangeMessage();

		currentTabChanged (newIndex, getCurrentTabName());
	}
}

WBTabBarButton* WBTabbedButtonBar::getTabButton (const int index) const
{
	TabInfo* const tab = tabs[index];
	return tab == nullptr ? nullptr : static_cast <WBTabBarButton*> (tab->component);
}

int WBTabbedButtonBar::indexOfTabButton (const WBTabBarButton* button) const
{
	for (int i = tabs.size(); --i >= 0;)
		if (tabs.getUnchecked(i)->component == button)
			return i;

	return -1;
}

void WBTabbedButtonBar::lookAndFeelChanged()
{
	extraTabsButton = nullptr;
	resized();
}

int WBTabbedButtonBar::getTabButtonOverlap (int tabDepth)
{
    return 1 + tabDepth/3;
}

void WBTabbedButtonBar::resized()
{
	int depth = getWidth();
	int length = getHeight();

	if (orientation == TabsAtTop || orientation == TabsAtBottom)
		std::swap (depth, length);

	const int overlap = getTabButtonOverlap (depth)
							+ getLookAndFeel().getTabButtonSpaceAroundImage() * 2;

	int i, totalLength = overlap;
	int numVisibleButtons = tabs.size();

	for (i = 0; i < tabs.size(); ++i)
	{
		WBTabBarButton* const tb = tabs.getUnchecked(i)->component;

		totalLength += tb->getBestTabLength (depth) - overlap;
		tb->overlapPixels = overlap / 2;
	}

	double scale = 1.0;

	if (totalLength > length)
		scale = jmax (minimumScale, length / (double) totalLength);

	const bool isTooBig = totalLength * scale > length;
	int tabsButtonPos = 0;

	if (isTooBig)
	{
		if (extraTabsButton == nullptr)
		{
			addAndMakeVisible (extraTabsButton = getLookAndFeel().createTabBarExtrasButton());
			extraTabsButton->addListener (behindFrontTab);
			extraTabsButton->setAlwaysOnTop (true);
			extraTabsButton->setTriggeredOnMouseDown (true);
		}

		const int buttonSize = jmin (proportionOfWidth (0.7f), proportionOfHeight (0.7f));
		extraTabsButton->setSize (buttonSize, buttonSize);

		if (orientation == TabsAtTop || orientation == TabsAtBottom)
		{
			tabsButtonPos = getWidth() - buttonSize / 2 - 1;
			extraTabsButton->setCentrePosition (tabsButtonPos, getHeight() / 2);
		}
		else
		{
			tabsButtonPos = getHeight() - buttonSize / 2 - 1;
			extraTabsButton->setCentrePosition (getWidth() / 2, tabsButtonPos);
		}

		totalLength = 0;

		for (i = 0; i < tabs.size(); ++i)
		{
			WBTabBarButton* const tb = tabs.getUnchecked(i)->component;

			const int newLength = totalLength + tb->getBestTabLength (depth);

			if (i > 0 && newLength * minimumScale > tabsButtonPos)
			{
				totalLength += overlap;
				break;
			}

			numVisibleButtons = i + 1;
			totalLength = newLength - overlap;
		}

		scale = jmax (minimumScale, tabsButtonPos / (double) totalLength);
	}
	else
	{
		extraTabsButton = nullptr;
	}

	int pos = 0;

	WBTabBarButton* frontTab = nullptr;

	for (i = 0; i < tabs.size(); ++i)
	{
		WBTabBarButton* const tb = getTabButton (i);

		if (tb != nullptr)
		{
			const int bestLength = roundToInt (scale * tb->getBestTabLength (depth));

			if (i < numVisibleButtons)
			{
				if (orientation == TabsAtTop || orientation == TabsAtBottom)
					tb->setBounds (pos, 0, bestLength, getHeight());
				else
					tb->setBounds (0, pos, getWidth(), bestLength);

				tb->toBack();

				if (i == currentTabIndex)
					frontTab = tb;

				tb->setVisible (true);
			}
			else
			{
				tb->setVisible (false);
			}

			pos += bestLength - overlap;
		}
	}

	behindFrontTab->setBounds (getLocalBounds());

	if (frontTab != nullptr)
	{
		frontTab->toFront (false);
		behindFrontTab->toBehind (frontTab);
	}
}

const Colour WBTabbedButtonBar::getTabBackgroundColour (const int tabIndex)
{
	TabInfo* const tab = tabs [tabIndex];
	return tab == nullptr ? Colours::white : tab->colour;
}

void WBTabbedButtonBar::setTabBackgroundColour (const int tabIndex, const Colour& newColour)
{
	TabInfo* const tab = tabs [tabIndex];

	if (tab != nullptr && tab->colour != newColour)
	{
		tab->colour = newColour;
		repaint();
	}
}

void WBTabbedButtonBar::extraItemsMenuCallback (int result, WBTabbedButtonBar* bar)
{
	if (bar != nullptr && result > 0)
		bar->setCurrentTabIndex (result - 1);
}

void WBTabbedButtonBar::showExtraItemsMenu()
{
	PopupMenu m;

	for (int i = 0; i < tabs.size(); ++i)
	{
		const TabInfo* const tab = tabs.getUnchecked(i);

		if (! tab->component->isVisible())
			m.addItem (i + 1, tab->name, true, i == currentTabIndex);
	}

	m.showMenuAsync (PopupMenu::Options().withTargetComponent (extraTabsButton),
					 ModalCallbackFunction::forComponent (extraItemsMenuCallback, this));
}

void WBTabbedButtonBar::currentTabChanged (const int, const String&)
{
}

void WBTabbedButtonBar::popupMenuClickOnTab (const int, const String&)
{
}


