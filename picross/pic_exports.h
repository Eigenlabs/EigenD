
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
  #ifdef BUILDING_PIC
    #define PIC_DECLSPEC_FUNC(rt) rt __declspec(dllexport)
    #define PIC_DECLSPEC_CLASS __declspec(dllexport)
  #else
    #define PIC_DECLSPEC_FUNC(rt) rt __declspec(dllimport)
    #define PIC_DECLSPEC_CLASS __declspec(dllimport)
  #endif
#else
  #if __GNUC__ >= 4
    #ifdef BUILDING_PIC
      #define PIC_DECLSPEC_FUNC(rt) rt __attribute__ ((visibility("default")))
      #define PIC_DECLSPEC_CLASS __attribute__ ((visibility("default")))
    #else
      #define PIC_DECLSPEC_FUNC(rt) rt  __attribute__ ((visibility("hidden")))
      #define PIC_DECLSPEC_CLASS  __attribute__ ((visibility("hidden")))
    #endif
  #else
    #define PIC_DECLSPEC_FUNC(rt) rt
    #define PIC_DECLSPEC_CLASS
  #endif
#endif
