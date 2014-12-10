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

#ifndef __RESIZABLE_LABEL__
#define __RESIZABLE_LABEL__


#include "WidgetComponent.h"

class WidgetComponent;

class ResizableLabel : public Label
{
public:
    ResizableLabel(const String &componentName, const String &labelText);

    void resized();
    
    void paint(Graphics& g);

    void drawFittedText (const Graphics& g, const String& text,
                         const int x, const int y, const int width, const int height,
                         const Justification& justification,
                         const int maximumNumberOfLines,
                         const float minimumHorizontalScale) const;

    void setVertical(bool isVertical, bool isUp)
    { 
        isVertical_ = isVertical;
        isUp_ = isUp;
        resized();
    }
    
    void setWidget(WidgetComponent* widget);
    String getTooltip();
    
private:
    // draw text vertically
    bool isVertical_;
    // draw vertical text from bottom to top
    bool isUp_;

    WidgetComponent* widget_;
};




#endif // __RESIZABLE_LABEL__
