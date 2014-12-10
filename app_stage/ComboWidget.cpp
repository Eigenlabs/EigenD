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

#include "ComboWidget.h"
#include "Terms.h"

static StringArray getChoices(const XmlElement *e)
{
    const XmlElement *domain = e->getChildByName("domain");
    StringArray retval;

    if(!domain)
    {
        return retval;
    }

    String full_domain = domain->getStringAttribute("full");
    term_t parsed_domain = parse_state_term(full_domain);

    if(!parsed_domain.is_pred() || parsed_domain.pred()!="enums" || parsed_domain.arity()<1)
    {
        return retval;
    }

    for(unsigned i=0;i<parsed_domain.arity();i++)
    {
        if(parsed_domain.arg(i).is_atom())
        {
            retval.add(parsed_domain.arg(i).value());
        }
    }

    return retval;
}

ComboWidget::ComboWidget(WidgetComponent *widget): EigenOSCWidget(widget)
{
    StringArray choices = getChoices(widget->getXml());

    addAndMakeVisible(choices_ = new ComboBox);

    choices_->setEditableText(false);

    for(int i=0;i<choices.size();i++)
    {
        choices_->addItem(choices[i],i+1);
    }

    choices_->setSelectedId(1);
    choices_->addListener(this);
}

ComboWidget::~ComboWidget()
{
    choices_->removeListener(this);
}

void ComboWidget::resized()
{
    //std::cout << "custom resized " << getWidth() << "x" << getHeight() << '\n';
    choices_->setBounds(getLocalBounds());
}

void ComboWidget::comboBoxChanged(ComboBox *combo)
{
    sendValue(WidgetData(choices_->getText()));
}

void ComboWidget::resetValue(bool is_latent, const WidgetData &current, const WidgetData &latent)
{
    if(is_latent)
    {
        choices_->setColour(ComboBox::textColourId,Colours::red);
    }
    else
    {
        choices_->setColour(ComboBox::textColourId,Colours::black);
    }

    choices_->setText(current.string_,true);
}

