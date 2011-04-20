
/*
 Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#include <picross/pic_power.h>
#include <picross/pic_log.h>

#ifdef PI_LINUX

void pic::display_active(void)
{
}

void pic::disk_active(void)
{
}

void pic::to_front(void)
{
}

#endif

#ifdef PI_MACOSX

#include <CoreServices/CoreServices.h>
#include <Carbon/Carbon.h>

void pic::disk_active(void)
{
    UpdateSystemActivity(HDActivity);
}

void pic::display_active(void)
{
    UpdateSystemActivity(UsrActivity);
}

void pic::to_front(void)
{
    ProcessSerialNumber psn;
    if(GetCurrentProcess(&psn)==noErr)
        SetFrontProcess(&psn);
}

#endif

#ifdef PI_WINDOWS
#pragma message ("      ****  Needs fixing for windows  ****")
void pic::display_active(void)
{
}

void pic::disk_active(void)
{
}

void pic::to_front(void)
{
}

#endif

