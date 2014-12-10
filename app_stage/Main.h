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

#ifndef __MAIN_H__
#define __MAIN_H__

#include "MainComponent.h"
#include "StagePreferences.h"

#include "juce.h"

//==============================================================================
/**
 This is the top-level window that we'll pop up. Inside it, we'll create and
 show a component from the MainComponent.cpp file (you can open this file using
 the Jucer to edit it).
 */

class MainComponent;
class StageApplication;

class StageWindow  : public DocumentWindow, public MessageListener
{
public:
    //==============================================================================
    StageWindow();
    ~StageWindow();
    void closeButtonPressed();
    void activeWindowStatusChanged();
    void broughtToFront();
    void suspend();
    void resume();
    
    void handleMessage(const Message& message);
 
    // Get around compiler bug causing bad delete
    void *dummy;
    // the command manager object used to dispatch command events
    ApplicationCommandManager* commandManager;
   
};


#endif // __MAIN_H__
