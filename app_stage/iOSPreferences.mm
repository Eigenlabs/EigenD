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

#import <UIKit/UIKit.h>

static const String nsStringToJuce (NSString* s)
{
	return String::fromUTF8([s UTF8String]);
}

static NSString* juceStringToNS (const String& s)
{
	return [NSString stringWithUTF8String: s.toUTF8()];
}

StagePreferences::StagePreferences() {}
StagePreferences::~StagePreferences() {}

void StagePreferences::setValue(const juce::String &keyName, const var &value)
{
	if (value.isBool())
	{
		[[NSUserDefaults standardUserDefaults] setBool: (bool)value forKey: juceStringToNS(keyName)];
	}
	else if (value.isInt())
	{
		[[NSUserDefaults standardUserDefaults] setInteger: (int)value forKey: juceStringToNS(keyName)];
	}
}

bool StagePreferences::getBoolValue(const String &keyName, const bool defaultReturnValue) const throw()
{
	if (nil == [[NSUserDefaults standardUserDefaults] objectForKey: juceStringToNS(keyName)])
	{
		return defaultReturnValue;
	}
	
	return [[NSUserDefaults standardUserDefaults] boolForKey: juceStringToNS(keyName)];
}

int StagePreferences::getIntValue(const String &keyName, const int defaultReturnValue) const throw()
{
	if (nil == [[NSUserDefaults standardUserDefaults] objectForKey: juceStringToNS(keyName)])
	{
		return defaultReturnValue;
	}
	
	return [[NSUserDefaults standardUserDefaults] integerForKey: juceStringToNS(keyName)];
}

bool StagePreferences::saveIfNeeded()
{
	return [[NSUserDefaults standardUserDefaults] synchronize];
}