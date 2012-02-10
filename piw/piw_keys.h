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

#ifndef __PIW_KEYS_
#define __PIW_KEYS_

#include "piw_exports.h"

#include <piw/piw_data.h>

namespace piw
{
    /**
     * Different hardness levels of a key press.
     *
     * It always starts off as a KEY_LIGHT press, potentially afterwards moving
     * towards either KEY_SOFT or KEY_HARD but never both
     */
    enum hardness_t {KEY_LIGHT, KEY_SOFT, KEY_HARD};

    /**
     * Create a new key data structure with the required elements.
     *
     * A key has two completely independent positioning schemes, the physical one
     * and the musical one. These can be used for different purposes and be remapped
     * independently. Each position is a coordinate from two axis, for the physical
     * position we call these axis the row and column, for the musical position we
     * call these axis the course and the key. Each position also has a sequential
     * number that is unique for this key. This allows for quick identification of
     * the key in either schemes and also to make it more intuitive to handle linear
     * layouts that should naturally flow across the axis in the correct order.
     *
     * The row, col, course and key parameters can be negative, which indicates that
     * they're offset from the opposite edge of their bounding geometry. Just as
     * the number 1 indicates for instance the first column on a particular row,
     * the number -1 will indicate the last column on that row. Similarly,
     * the number 2 will indiciate the second column and -2 will indicate the
     * before last column, and so on.
     *
     * @param pseq      the sequential position in the physical layout
     * @param row       the row index in the physical layout
     * @param col       the column index in the physical layout
     * @param mseq      the sequential position in the musical layout
     * @param course    the course index in the musical layout
     * @param key       the key index in the musical layout
     * @param hardness  the hardness level of the key press
     * @param t         the timestamp of the data object
     *
     * @returns a new data instance with the provided key parameters
     */
    PIW_DECLSPEC_FUNC(piw::data_nb_t) makekey(unsigned pseq, int row, int col, unsigned mseq, int course, int key, hardness_t hardness, unsigned long long t);

    /**
     * Checks if the provided data contains valid key information.
     *
     * This method merely checks if the structure of the data can be used to extract
     * key information from, it doesn't however check the validity nor the existence.
     *
     * @param d  the data instance that will be checked
     *
     * @returns true if the provided data instance is a key; or
     *          false otherwise
     */
    PIW_DECLSPEC_FUNC(bool) is_key(const piw::data_t &d);

    /**
     * Decodes the provided data instance into individual parts that contain
     * the key information
     *
     * Note that any of the parameters apart from the data instance are optional.
     * They are all pointers towards variables that will be filled with the key
     * information that was extracted from the data instance.
     * If they're zero, they will not be filled in. This can be used to extract
     * information selectively.
     *
     * For more information about the purpose of each parameter
     * @see makekey
     *
     * @returns true if the data was successfully decoded into key information; or
     *          false if the data couldn't be decoded
     */
    PIW_DECLSPEC_FUNC(bool) decode_key(const piw::data_nb_t &d, unsigned *pseq=0, float *row=0, float *col=0, unsigned *mseq=0, float *course=0, float *key=0, hardness_t *hardness=0);

    /**
     * Calculates the sequential position of a key, based on its coordinates.
     *
     * By using the lengths of the rows (physical) or the courses (musical) and
     * the coordinates of the key position, the sequential position of the key
     * will be calculated.
     *
     * @param lengths  a tuple data instance that contains the lengths of the
     *                 rows or course as longs
     * @param x        the first part of the coordinate (row or course)
     * @param y        the second part of the coordinate (column or key)
     *
     * @see makekey
     *
     * @returns the positive sequential position of the key;
     *          or 0 if it couldn't be calculated
     */
    PIW_DECLSPEC_FUNC(unsigned) key_sequential(const piw::data_t &lengths, int x, int y);

    /**
     * Calculates the coordinates of a key, based on its sequential position.
     *
     * By using the lengths of the rows (physical) or the courses (musical) and
     * the sequential key position, the coordinate of the key will be calculated.
     *
     *
     * @param lengths  a tuple data instance that contains the lengths of the
     *                 rows or course as longs
     * @param x        a pointer to a variable that will be filled with the first
     *                 part of the coordinate (row or course), if the coordinate
     *                 can't be calculated the variable will be set to 0
     * @param y        a pointer to a variable that will be filled with the second
     *                 part of the coordinate (column or key), if the coordinate
     *                 can't be calculated the variable will be set to 0
     *
     * @see makekey
     */
    PIW_DECLSPEC_FUNC(void) key_coordinates(unsigned sequential, const piw::data_nb_t &lengths, int *x, int *y);
};

#endif
