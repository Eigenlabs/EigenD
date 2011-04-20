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

#ifndef __PIA_SRC_ERROR__
#define __PIA_SRC_ERROR__

#include <picross/pic_log.h>

#define PIA_CATCHLOG_GLUE(glue) \
    catch(std::exception &__e) \
    { \
        pic::msg() << __e.what() << " caught at " << __FILE__ << ':' << __LINE__ << (glue); \
    } \
    catch(const char *__e) \
    { \
        pic::msg() << __e << " caught at " << __FILE__ << ':' << __LINE__ << (glue); \
    } \
    catch(...) \
    { \
        pic::msg() << "unknown exception caught at " << __FILE__ << ':' << __LINE__ << (glue); \
    }


#define PIA_CATCHLOG_PRINT() \
    catch(std::exception &__e) \
    { \
        pic::msg() << __e.what() << " caught at " << __FILE__ << ':' << __LINE__ << pic::print; \
    } \
    catch(const char *__e) \
    { \
        pic::msg() << __e << " caught at " << __FILE__ << ':' << __LINE__ << pic::print; \
    } \
    catch(...) \
    { \
        pic::msg() << "unknown exception caught at " << __FILE__ << ':' << __LINE__ << pic::print; \
    }

#define PIA_CATCHLOG_EREF(ctx) PIA_CATCHLOG_PRINT()

#endif
