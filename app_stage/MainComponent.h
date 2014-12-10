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

#ifndef __MAIN_COMPONENT_H__
#define __MAIN_COMPONENT_H__

#define DESKTOP 1
#define IOS 2

// normally defined by compiler flags depending on the project

//#define STAGE_BUILD DESKTOP
//#define STAGE_BUILD IOS


#if STAGE_BUILD==DESKTOP
#include "DesktopMainComponent.h"
#elif STAGE_BUILD==IOS
#include "iOSMainComponent.h"
#else
#include "DesktopMainComponent.h"
#endif // STAGE_BUILD==1


#endif // __MAIN_COMPONENT_H__

