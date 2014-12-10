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

# include "juce.h"
# include <iostream>
# include "WorkbenchColours.h"

void drawTrunkBody(Graphics& g, float zoomFactor, int length, int thickness, int orientation, int x, int y,bool mouseOver);
void drawHorizontalTrunkBody(Graphics& g, float zoomFactor, int length, int thickness, int x, int y,bool mouseOver);
void drawVerticalTrunkBody(Graphics& g, float zoomFactor, int length, int thickness, int x, int y,bool mouseOver);
Path getTrunkOutline(float zoomFactor, int length, int thickness);
Path getTrueTrunkOutline(int length, int thickness);
void drawTrunkBody(Graphics& g, float zoomFactor, int length, int thickness, int x, int y,bool mouseOver,bool vertical);
