
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

#ifdef _WIN32
  #ifdef BUILDING_PIAUDIO
    #define PIAUDIO_DECLSPEC_FUNC(rt) rt __declspec(dllexport)
    #define PIAUDIO_DECLSPEC_CLASS __declspec(dllexport)
  #else
    #define PIAUDIO_DECLSPEC_FUNC(rt) rt __declspec(dllimport)
    #define PIAUDIO_DECLSPEC_CLASS __declspec(dllimport)
  #endif
#else
  #if __GNUC__ >= 4
    #ifdef BUILDING_PIAUDIO
      #define PIAUDIO_DECLSPEC_FUNC(rt) rt __attribute__ ((visibility("default")))
      #define PIAUDIO_DECLSPEC_CLASS __attribute__ ((visibility("default")))
      #ifndef JUCE_API
          #define JUCE_API __attribute__ ((visibility("default")))
      #endif
    #else
      #define PIAUDIO_DECLSPEC_FUNC(rt) rt  __attribute__ ((visibility("hidden")))
      #define PIAUDIO_DECLSPEC_CLASS  __attribute__ ((visibility("hidden")))
      #ifndef JUCE_API
          #define JUCE_API __attribute__ ((visibility("hidden")))
      #endif
    #endif
  #else
    #define PIAUDIO_DECLSPEC_FUNC(rt) rt
    #define PIAUDIO_DECLSPEC_CLASS
  #endif
#endif
