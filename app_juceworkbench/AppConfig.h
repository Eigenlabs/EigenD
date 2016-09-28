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

#define JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED 1
#define JUCE_MODULE_AVAILABLE_juce_core                 1
#define JUCE_MODULE_AVAILABLE_juce_data_structures      1
#define JUCE_MODULE_AVAILABLE_juce_events               1
#define JUCE_MODULE_AVAILABLE_juce_graphics             1
#define JUCE_MODULE_AVAILABLE_juce_gui_basics           1

#define JUCE_ASIO 0
#define JUCE_WASAPI 0
#define JUCE_DIRECTSOUND 0
#define JUCE_ALSA 0
#define JUCE_JACK 0
#define JUCE_USE_ANDROID_OPENSLES 0
#define JUCE_USE_CDREADER 0
#define JUCE_USE_CDBURNER 0

#define JUCE_USE_FLAC 0
#define JUCE_USE_OGGVORBIS 0
#define JUCE_USE_MP3AUDIOFORMAT 0
#define JUCE_USE_WINDOWS_MEDIA_FORMAT 0

#define JUCE_PLUGINHOST_VST 0
#define JUCE_PLUGINHOST_AU 0

// #define JUCE_FORCE_DEBUG 1
// #define JUCE_LOG_ASSERTIONS 1
// #define JUCE_CHECK_MEMORY_LEAKS 1
#define JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES 1

#define JUCE_USE_COREIMAGE_LOADER 0
#define JUCE_USE_DIRECTWRITE 0

#define JUCE_ENABLE_REPAINT_DEBUGGING 0
#ifdef PI_LINUX_ARMV7L
#define JUCE_USE_XSHM 0
#define JUCE_USE_XINERAMA 0
#else
#define JUCE_USE_XSHM 1
#endif

#define JUCE_USE_XRENDER 0
#define JUCE_USE_XCURSOR 0

#define JUCE_WEB_BROWSER 0

#define JUCE_DIRECTSHOW 0
#define JUCE_MEDIAFOUNDATION 0
#define JUCE_QUICKTIME 0
#define JUCE_USE_CAMERA 0
