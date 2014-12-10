/*
 Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#include "ejuce_laf.h"
#include <picross/pic_config.h>

ejuce::EJuceLookandFeel::EJuceLookandFeel()
{
#ifdef PI_MACOSX
    setDefaultSansSerifTypefaceName("AppleGothic");
#endif

#ifdef PI_WINDOWS
    setDefaultSansSerifTypefaceName("MS Gothic UI");
#endif

    setColour(juce::TextEditor::focusedOutlineColourId,juce::Colour(0x00000000));
    setColour(juce::TextEditor::outlineColourId,juce::Colour(0x00000000));
    setColour(juce::ComboBox::outlineColourId,juce::Colour(0x00000000));
    setColour(juce::ComboBox::buttonColourId,juce::Colour(0xffacacac));
    setColour(juce::TextButton::buttonColourId,juce::Colour(0xffacacac));
}

ejuce::EJuceLookandFeel::~EJuceLookandFeel()
{
}
