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

#ifndef __JUCER_GROUPINFORMATIONCOMPONENT_JUCEHEADER__
#define __JUCER_GROUPINFORMATIONCOMPONENT_JUCEHEADER__

#include "../jucer_Headers.h"
#include "../Project/jucer_Project.h"


//==============================================================================
class GroupInformationComponent  : public Component,
                                   private ListBoxModel,
                                   private ValueTree::Listener
{
public:
    GroupInformationComponent (const Project::Item& group)
        : item (group)
    {
        list.setModel (this);
        list.setColour (ListBox::backgroundColourId, Colours::transparentBlack);
        addAndMakeVisible (&list);
        list.updateContent();
        list.setRowHeight (20);
        item.state.addListener (this);
        lookAndFeelChanged();
    }

    ~GroupInformationComponent()
    {
        item.state.removeListener (this);
    }

    //==============================================================================
    void paint (Graphics& g)
    {
        IntrojucerLookAndFeel::fillWithBackgroundTexture (*this, g);
    }

    void resized()
    {
        list.setBounds (getLocalBounds().reduced (5, 4));
    }

    int getNumRows()
    {
        return item.getNumChildren();
    }

    void paintListBoxItem (int /*rowNumber*/, Graphics& g, int width, int height, bool /*rowIsSelected*/)
    {
        g.setColour (Colours::white.withAlpha (0.4f));
        g.fillRect (0, 0, width, height - 1);
    }

    Component* refreshComponentForRow (int rowNumber, bool /*isRowSelected*/, Component* existingComponentToUpdate)
    {
        if (rowNumber < getNumRows())
        {
            Project::Item child (item.getChild (rowNumber));

            if (existingComponentToUpdate == nullptr
                 || dynamic_cast <FileOptionComponent*> (existingComponentToUpdate)->item != child)
            {
                delete existingComponentToUpdate;
                existingComponentToUpdate = new FileOptionComponent (child);
            }
        }
        else
        {
            deleteAndZero (existingComponentToUpdate);
        }

        return existingComponentToUpdate;
    }

    //==============================================================================
    void valueTreePropertyChanged (ValueTree&, const Identifier&)    { itemChanged(); }
    void valueTreeChildAdded (ValueTree&, ValueTree&)                { itemChanged(); }
    void valueTreeChildRemoved (ValueTree&, ValueTree&)              { itemChanged(); }
    void valueTreeChildOrderChanged (ValueTree&)                     { itemChanged(); }
    void valueTreeParentChanged (ValueTree&)                         { itemChanged(); }

private:
    Project::Item item;
    ListBox list;

    void itemChanged()
    {
        list.updateContent();
        repaint();
    }

    //==============================================================================
    class FileOptionComponent  : public Component
    {
    public:
        FileOptionComponent (const Project::Item& fileItem)
            : item (fileItem),
              compileButton ("Compile"),
              resourceButton ("Add to Binary Resources")
        {
            if (item.isFile())
            {
                addAndMakeVisible (&compileButton);
                compileButton.getToggleStateValue().referTo (item.getShouldCompileValue());

                addAndMakeVisible (&resourceButton);
                resourceButton.getToggleStateValue().referTo (item.getShouldAddToResourceValue());
            }
        }

        void paint (Graphics& g)
        {
            int x = getHeight() + 6;

            item.getIcon().withContrastingColourTo (Colours::grey)
                .draw (g, Rectangle<float> (3.0f, 2.0f, x - 6.0f, getHeight() - 4.0f),
                       item.isIconCrossedOut());

            g.setColour (Colours::black);
            g.setFont (getHeight() * 0.6f);

            const int x2 = compileButton.isVisible() ? compileButton.getX() - 4
                                                     : getWidth() - 4;

            g.drawText (item.getName(), x, 0, x2 - x, getHeight(), Justification::centredLeft, true);
        }

        void resized()
        {
            int w = 180;
            resourceButton.setBounds (getWidth() - w, 1, w, getHeight() - 2);
            w = 100;
            compileButton.setBounds (resourceButton.getX() - w, 1, w, getHeight() - 2);
        }

        Project::Item item;

    private:
        ToggleButton compileButton, resourceButton;
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GroupInformationComponent)
};


#endif   // __JUCER_GROUPINFORMATIONCOMPONENT_JUCEHEADER__
