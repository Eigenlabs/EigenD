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

#include "EigenWidgets.h"
#include "ComboWidget.h"
#include "ConductorWidgets.h"

EigenCustomWidget *EigenCustomWidget::widgetFactory(const String &widgetType, WidgetComponent *widget)
{
    if(widgetType=="custom-combo")
    {
        return new ComboWidget(widget);
    }

    if(widgetType=="custom-clipmanager")
    {
        return new ConductorClipManagerWidget(widget);
    }

    if(widgetType=="custom-clippool")
    {
        return new ConductorClipPoolWidget(widget);
    }

    if(widgetType=="custom-arrangeview")
    {
        return new ConductorArrangementViewWidget(widget);
    }

    if(widgetType=="custom-sceneview")
    {
        return new ConductorSceneViewWidget(widget);
    }

    return 0;
}
