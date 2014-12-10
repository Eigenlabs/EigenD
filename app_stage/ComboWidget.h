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

#ifndef __COMBO_WIDGET__
#define __COMBO_WIDGET__

#include "WidgetComponent.h"

class ComboWidget: public EigenOSCWidget, public ComboBox::Listener
{
    public:
        ComboWidget(WidgetComponent *widget);
        ~ComboWidget();
        void resized();
        void resetValue(bool is_latent, const WidgetData &current, const WidgetData &latent);
        void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
        float getGraphicsWidth() { return 100.0; }
        float getGraphicsHeight() { return 20.0; }

    private:
        WidgetComponent* widget_;
        ScopedPointer<ComboBox> choices_;
};

#endif
