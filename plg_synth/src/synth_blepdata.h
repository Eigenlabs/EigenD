
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

#ifndef BLEPDATA_H
#define BLEPDATA_H

#define TABLE_OVERSAMPLE 128
#define TABLE_OVERSAMPLEF 128.0f
#define TABLE_SAMPLES 142
#define TABLE_SAMPLESF 142.0f
#define TABLE_SIZE (TABLE_OVERSAMPLE*TABLE_SAMPLES)
#define TABLE_DELAY 4

extern float blepstep__[];
extern float blepdelta__[];
extern float blepcusp__[];
extern float blepcdelta__[];

#endif
