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

#ifndef __WIREPROP__
#define __WIREPROP__

#include "Wire.h"
#include <set>
#include "juce.h"
#include <vector>
#include "WireEditor.h"
#include "DuplicatePanel.h"

class WireEditor;

class WirePropertyPanel: public Component
{

public:
    WirePropertyPanel(std::vector<Wire*> wires);
    ~WirePropertyPanel();
    void resized();
    std::vector<WireEditor*> getEditors();
    virtual void handleCommandMessage(int commandId);

private:
    void addWireEditor(Wire* w);
    std::vector<Wire*> wires_;
    std::vector<WireEditor*> editors_;
    DuplicatePanel* dp_;
    void enableAllUsing(bool enable);
    void enableAllFilter(bool enable);
    void controlUsing();
    void controlFilter();
};



#endif
