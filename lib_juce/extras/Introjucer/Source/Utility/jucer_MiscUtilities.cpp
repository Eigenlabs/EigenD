/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#include "../jucer_Headers.h"


//==============================================================================
String createAlphaNumericUID()
{
    String uid;
    const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    Random r;

    uid << chars [r.nextInt (52)]; // make sure the first character is always a letter

    for (int i = 5; --i >= 0;)
    {
        r.setSeedRandomly();
        uid << chars [r.nextInt (62)];
    }

    return uid;
}

String hexString8Digits (int value)
{
    return String::toHexString (value).paddedLeft ('0', 8);
}

String createGUID (const String& seed)
{
    const String hex (MD5 ((seed + "_guidsalt").toUTF8()).toHexString().toUpperCase());

    return "{" + hex.substring (0, 8)
         + "-" + hex.substring (8, 12)
         + "-" + hex.substring (12, 16)
         + "-" + hex.substring (16, 20)
         + "-" + hex.substring (20, 32)
         + "}";
}

String escapeSpaces (const String& s)
{
    return s.replace (" ", "\\ ");
}

String addQuotesIfContainsSpaces (const String& text)
{
    return (text.containsChar (' ') && ! text.isQuotedString()) ? text.quoted() : text;
}

void setValueIfVoid (Value value, const var& defaultValue)
{
    if (value.getValue().isVoid())
        value = defaultValue;
}

//==============================================================================
StringPairArray parsePreprocessorDefs (const String& text)
{
    StringPairArray result;
    String::CharPointerType s (text.getCharPointer());

    while (! s.isEmpty())
    {
        String token, value;
        s = s.findEndOfWhitespace();

        while ((! s.isEmpty()) && *s != '=' && ! s.isWhitespace())
            token << s.getAndAdvance();

        s = s.findEndOfWhitespace();

        if (*s == '=')
        {
            ++s;

            s = s.findEndOfWhitespace();

            while ((! s.isEmpty()) && ! s.isWhitespace())
            {
                if (*s == ',')
                {
                    ++s;
                    break;
                }

                if (*s == '\\' && (s[1] == ' ' || s[1] == ','))
                    ++s;

                value << s.getAndAdvance();
            }
        }

        if (token.isNotEmpty())
            result.set (token, value);
    }

    return result;
}

StringPairArray mergePreprocessorDefs (StringPairArray inheritedDefs, const StringPairArray& overridingDefs)
{
    for (int i = 0; i < overridingDefs.size(); ++i)
        inheritedDefs.set (overridingDefs.getAllKeys()[i], overridingDefs.getAllValues()[i]);

    return inheritedDefs;
}

String createGCCPreprocessorFlags (const StringPairArray& defs)
{
    String s;

    for (int i = 0; i < defs.size(); ++i)
    {
        String def (defs.getAllKeys()[i]);
        const String value (defs.getAllValues()[i]);
        if (value.isNotEmpty())
            def << "=" << value;

        if (! def.endsWithChar ('"'))
            def = def.quoted();

        s += " -D " + def;
    }

    return s;
}

String replacePreprocessorDefs (const StringPairArray& definitions, String sourceString)
{
    for (int i = 0; i < definitions.size(); ++i)
    {
        const String key (definitions.getAllKeys()[i]);
        const String value (definitions.getAllValues()[i]);

        sourceString = sourceString.replace ("${" + key + "}", value);
    }

    return sourceString;
}

StringArray getSearchPathsFromString (const String& searchPath)
{
    StringArray s;
    s.addTokens (searchPath, ";\r\n", String::empty);
    s.trim();
    s.removeEmptyStrings();
    s.removeDuplicates (false);
    return s;
}

void addPlistDictionaryKey (XmlElement* xml, const String& key, const String& value)
{
    forEachXmlChildElementWithTagName (*xml, e, "key")
    {
        if (e->getAllSubText().trim().equalsIgnoreCase (key))
        {
            if (e->getNextElement() != nullptr && e->getNextElement()->hasTagName ("key"))
            {
                // try to fix broken plist format..
                xml->removeChildElement (e, true);
                break;
            }
            else
            {
                return; // (value already exists)
            }
        }
    }

    xml->createNewChildElement ("key")   ->addTextElement (key);
    xml->createNewChildElement ("string")->addTextElement (value);
}

void addPlistDictionaryKeyBool (XmlElement* xml, const String& key, const bool value)
{
    xml->createNewChildElement ("key")->addTextElement (key);
    xml->createNewChildElement (value ? "true" : "false");
}

void addPlistDictionaryKeyInt (XmlElement* xml, const String& key, int value)
{
    xml->createNewChildElement ("key")->addTextElement (key);
    xml->createNewChildElement ("integer")->addTextElement (String (value));
}

//==============================================================================
void autoScrollForMouseEvent (const MouseEvent& e, bool scrollX, bool scrollY)
{
    if (Viewport* const viewport = e.eventComponent->findParentComponentOfClass<Viewport>())
    {
        const MouseEvent e2 (e.getEventRelativeTo (viewport));
        viewport->autoScroll (scrollX ? e2.x : 20, scrollY ? e2.y : 20, 8, 16);
    }
}

//==============================================================================
int indexOfLineStartingWith (const StringArray& lines, const String& text, int index)
{
    const int len = text.length();

    for (const String* i = lines.begin() + index, * const e = lines.end(); i < e; ++i)
    {
        if (CharacterFunctions::compareUpTo (i->getCharPointer().findEndOfWhitespace(),
                                             text.getCharPointer(), len) == 0)
            return index;

        ++index;
    }

    return -1;
}

//==============================================================================
RolloverHelpComp::RolloverHelpComp()
    : lastComp (nullptr)
{
    setInterceptsMouseClicks (false, false);
    startTimer (150);
}

void RolloverHelpComp::paint (Graphics& g)
{
    AttributedString s;
    s.setJustification (Justification::centredLeft);
    s.append (lastTip, Font (14.0f), findColour (mainBackgroundColourId).contrasting (0.7f));

    TextLayout tl;
    tl.createLayoutWithBalancedLineLengths (s, getWidth() - 10.0f);
    if (tl.getNumLines() > 3)
        tl.createLayout (s, getWidth() - 10.0f);

    tl.draw (g, getLocalBounds().toFloat());
}

void RolloverHelpComp::timerCallback()
{
    Component* newComp = Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();

    if (newComp != nullptr
         && (newComp->getTopLevelComponent() != getTopLevelComponent()
              || newComp->isCurrentlyBlockedByAnotherModalComponent()))
        newComp = nullptr;

    if (newComp != lastComp)
    {
        lastComp = newComp;

        String newTip (findTip (newComp));

        if (newTip != lastTip)
        {
            lastTip = newTip;
            repaint();
        }
    }
}

String RolloverHelpComp::findTip (Component* c)
{
    while (c != nullptr)
    {
        if (TooltipClient* const tc = dynamic_cast <TooltipClient*> (c))
        {
            const String tip (tc->getTooltip());

            if (tip.isNotEmpty())
                return tip;
        }

        c = c->getParentComponent();
    }

    return String::empty;
}

//==============================================================================
FloatingLabelComponent::FloatingLabelComponent()
    : font (10.0f)
{
    setInterceptsMouseClicks (false, false);
}

void FloatingLabelComponent::remove()
{
    if (Component* p = getParentComponent())
        p->removeChildComponent (this);
}

void FloatingLabelComponent::update (Component* parent, const String& text, const Colour& textColour,
                                     int x, int y, bool toRight, bool below)
{
    colour = textColour;

    Rectangle<int> r;

    if (text != getName())
    {
        setName (text);
        glyphs.clear();
        glyphs.addJustifiedText (font, text, 0, 0, 200.0f, Justification::left);
        glyphs.justifyGlyphs (0, std::numeric_limits<int>::max(), 0, 0, 1000, 1000, Justification::topLeft);

        r = glyphs.getBoundingBox (0, std::numeric_limits<int>::max(), false)
                  .getSmallestIntegerContainer().expanded (1, 1);
    }
    else
    {
        r = getLocalBounds();
    }

    r.setPosition (x + (toRight ? 3 : -(r.getWidth() + 3)), y + (below ? 2 : -(r.getHeight() + 2)));
    setBounds (r);
    parent->addAndMakeVisible (this);
}

void FloatingLabelComponent::paint (Graphics& g)
{
    g.setFont (font);
    g.setColour (Colours::white.withAlpha (0.5f));
    g.fillRoundedRectangle (0, 0, (float) getWidth(), (float) getHeight(), 3);

    g.setColour (colour);
    glyphs.draw (g, AffineTransform::translation (1.0f, 1.0f));
}


//==============================================================================
class UTF8Component  : public Component,
                       private TextEditorListener
{
public:
    UTF8Component()
        : desc (String::empty,
                "Type any string into the box, and it'll be shown below as a portable UTF-8 literal, "
                "ready to cut-and-paste into your source-code...")
    {
        desc.setJustificationType (Justification::centred);
        desc.setColour (Label::textColourId, Colours::white);
        addAndMakeVisible (&desc);

        const Colour bkgd (Colours::white.withAlpha (0.6f));

        userText.setMultiLine (true, true);
        userText.setReturnKeyStartsNewLine (true);
        userText.setColour (TextEditor::backgroundColourId, bkgd);
        addAndMakeVisible (&userText);
        userText.addListener (this);

        resultText.setMultiLine (true, true);
        resultText.setColour (TextEditor::backgroundColourId, bkgd);
        resultText.setReadOnly (true);
        resultText.setSelectAllWhenFocused (true);
        addAndMakeVisible (&resultText);

        userText.setText (getLastText());
    }

    void textEditorTextChanged (TextEditor&)
    {
        update();
    }

    void textEditorEscapeKeyPressed (TextEditor&)
    {
        getTopLevelComponent()->exitModalState (0);
    }

    void update()
    {
        getLastText() = userText.getText();
        resultText.setText (CodeHelpers::stringLiteral (getLastText(), 100), false);
    }

    void resized()
    {
        desc.setBounds (8, 8, getWidth() - 16, 44);
        userText.setBounds (desc.getX(), desc.getBottom() + 8, getWidth() - 16, getHeight() / 2 - desc.getBottom() - 8);
        resultText.setBounds (desc.getX(), userText.getBottom() + 4, getWidth() - 16, getHeight() - userText.getBottom() - 12);
    }

private:
    Label desc;
    TextEditor userText, resultText;

    String& getLastText()
    {
        static String t;
        return t;
    }
};

void showUTF8ToolWindow (ScopedPointer<Component>& ownerPointer)
{
    if (ownerPointer != nullptr)
    {
        ownerPointer->toFront (true);
    }
    else
    {
        new FloatingToolWindow ("UTF-8 String Literal Converter",
                                "utf8WindowPos",
                                new UTF8Component(), ownerPointer,
                                400, 300,
                                300, 300, 1000, 1000);
    }
}

//==============================================================================
bool cancelAnyModalComponents()
{
    ModalComponentManager& mm = *ModalComponentManager::getInstance();
    const int numModal = mm.getNumModalComponents();

    for (int i = numModal; --i >= 0;)
        if (Component* c = mm.getModalComponent(i))
            c->exitModalState (0);

    return numModal > 0;
}

class AsyncCommandRetrier  : public Timer
{
public:
    AsyncCommandRetrier (const ApplicationCommandTarget::InvocationInfo& inf)
        : info (inf)
    {
        info.originatingComponent = nullptr;
        startTimer (500);
    }

    void timerCallback()
    {
        stopTimer();
        commandManager->invoke (info, true);
        delete this;
    }

    ApplicationCommandTarget::InvocationInfo info;

    JUCE_DECLARE_NON_COPYABLE (AsyncCommandRetrier)
};

bool reinvokeCommandAfterCancellingModalComps (const ApplicationCommandTarget::InvocationInfo& info)
{
    if (cancelAnyModalComponents())
    {
        new AsyncCommandRetrier (info);
        return true;
    }

    return false;
}
