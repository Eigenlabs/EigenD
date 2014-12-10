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

#ifndef __EIGEN_DIALOG_WINDOW_H__
#define __EIGEN_DIALOG_WINDOW_H__

#include "juce.h"

//------------------------------------------------------------------------------
// EigenDialogWindow
//
// Class that provides transparent titlebarless window for showing dialogs
//------------------------------------------------------------------------------

class EigenDialogWindow : public DialogWindow
{
public:    
    EigenDialogWindow(String name)
        : DialogWindow(name, Colour(0x00), false, true)
    {
        setUsingNativeTitleBar(false);
        // this is the only way to not have a title bar
        setTitleBarHeight(0);
        setAlwaysOnTop(true);
#if WIN32
        setDropShadowEnabled(false);
#endif // WIN32
    }
    
    void closeButtonPressed()
    {
        // no close button so do nothing, but have to implement this
    }
    
    virtual BorderSize<int> getBorderThickness ()
    {
        // return null border size for no border
        return BorderSize<int>();
    }

//    void resized()
//    {
//        // log the size
//        std::cout << "size=(" << getWidth() << ", " << getHeight() << ")\n";
//        DialogWindow::resized();
//    }
    
private:
    EigenDialogWindow (const EigenDialogWindow&);
    EigenDialogWindow& operator= (const EigenDialogWindow&);
    
};

#endif // __EIGEN_DIALOG_WINDOW_H__