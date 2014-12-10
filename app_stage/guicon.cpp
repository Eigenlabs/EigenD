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

#include <windows.h>

#include <stdio.h>

#include <fcntl.h>

#include <io.h>

#include <iostream>

#include <fstream>

#ifndef _USE_OLD_IOSTREAMS

using namespace std;

#endif

// maximum mumber of lines the output console should have

static const WORD MAX_CONSOLE_LINES = 500;

#ifdef _DEBUG

void RedirectIOToConsole()

{

int hConHandle;

long lStdHandle;

CONSOLE_SCREEN_BUFFER_INFO coninfo;

FILE *fp;

// allocate a console for this app

AllocConsole();

// set the screen buffer to be big enough to let us scroll text

GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),

&coninfo);

coninfo.dwSize.Y = MAX_CONSOLE_LINES;

SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),

coninfo.dwSize);

// redirect unbuffered STDOUT to the console

lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);

hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

fp = _fdopen( hConHandle, "w" );

*stdout = *fp;

setvbuf( stdout, NULL, _IONBF, 0 );

// redirect unbuffered STDIN to the console

lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);

hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

fp = _fdopen( hConHandle, "r" );

*stdin = *fp;

setvbuf( stdin, NULL, _IONBF, 0 );

// redirect unbuffered STDERR to the console

lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);

hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

fp = _fdopen( hConHandle, "w" );

*stderr = *fp;

setvbuf( stderr, NULL, _IONBF, 0 );

// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog

// point to console as well

ios::sync_with_stdio();

}

#endif

//End of File