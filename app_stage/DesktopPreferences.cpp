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

#include "StagePreferences.h"

StagePreferences::StagePreferences()
{
    PropertiesFile::Options options;
    options.applicationName     = "Stage";
    options.filenameSuffix      = "pref";
    options.osxLibrarySubFolder = "Application Support";
    options.folderName          = "";

    preferences_ = new PropertiesFile (options);
}

StagePreferences::~StagePreferences()
{
    if (preferences_) 
    {
        preferences_->saveIfNeeded();
        deleteAndZero(preferences_);
    }
}

void StagePreferences::setValue(const juce::String &keyName, const var &value)
{
    preferences_->setValue(keyName, value);
}

bool StagePreferences::getBoolValue(const String &keyName, const bool defaultReturnValue) const throw()
{
    return preferences_->getBoolValue(keyName, defaultReturnValue);
}

int StagePreferences::getIntValue(const String &keyName, const int defaultReturnValue) const throw()
{
    return preferences_->getIntValue(keyName, defaultReturnValue);
}

bool StagePreferences::saveIfNeeded()
{
    return preferences_->saveIfNeeded();
}

