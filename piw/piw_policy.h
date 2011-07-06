
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

#ifndef __PIW_POLICY__
#define __PIW_POLICY__

#include "piw_exports.h"
#include <picross/pic_ref.h>
#include <piw/piw_dataqueue.h>

#define TICK_DISABLE (-1) // don't tick me any more
#define TICK_SUPPRESS (0) // suppress but wake up on new data
#define TICK_ENABLE (1)   // keep on ticking

namespace piw
{
    struct PIW_DECLSPEC_CLASS converter_t: virtual public pic::counted_t, virtual public pic::lckobject_t
    {
        virtual dataqueue_t convert(const dataqueue_t &input,unsigned long long t) = 0;
        virtual int ticked(unsigned long long from,unsigned long long to, unsigned long sr, unsigned bs) = 0;
        virtual void clock_status(bool) {}
        virtual void flush() {}
    };

    typedef pic::ref_t<converter_t> converter_ref_t;

    PIW_DECLSPEC_FUNC(converter_ref_t) null_converter();
    PIW_DECLSPEC_FUNC(converter_ref_t) interpolating_converter(float u,float l,float z);
    PIW_DECLSPEC_FUNC(converter_ref_t) lopass_converter(float f, float c);
    PIW_DECLSPEC_FUNC(converter_ref_t) resampling_converter();
    PIW_DECLSPEC_FUNC(converter_ref_t) throttling_converter(unsigned long long interval);
    PIW_DECLSPEC_FUNC(converter_ref_t) triggering_converter();
    PIW_DECLSPEC_FUNC(converter_ref_t) impulse_converter();
    PIW_DECLSPEC_FUNC(converter_ref_t) filtering_converter(const piw::d2b_nb_t &);

    PIW_DECLSPEC_FUNC(d2d_nb_t) root_filter();
    PIW_DECLSPEC_FUNC(d2d_nb_t) null_filter();
    PIW_DECLSPEC_FUNC(d2d_nb_t) signal_dsc_filter(unsigned,unsigned,const char *);
    PIW_DECLSPEC_FUNC(d2d_nb_t) signal_cnc_filter(unsigned,unsigned);
    PIW_DECLSPEC_FUNC(d2d_nb_t) aggregation_filter(unsigned);
    PIW_DECLSPEC_FUNC(d2d_nb_t) aggregation_filter3(unsigned,unsigned,unsigned);
    PIW_DECLSPEC_FUNC(d2d_nb_t) deaggregation_filter(unsigned);
    PIW_DECLSPEC_FUNC(d2d_nb_t) grist_aggregation_filter(unsigned);
    PIW_DECLSPEC_FUNC(d2d_nb_t) gristchaff_aggregation_filter(unsigned,unsigned);
    PIW_DECLSPEC_FUNC(d2d_nb_t) grist_deaggregation_filter(unsigned);
    PIW_DECLSPEC_FUNC(d2d_nb_t) grist_deaggregation_filter2(unsigned,unsigned);
    PIW_DECLSPEC_FUNC(d2d_nb_t) last_lt_filter(unsigned);
    PIW_DECLSPEC_FUNC(d2d_nb_t) last_gt_filter(unsigned);
    PIW_DECLSPEC_FUNC(d2d_nb_t) last_filter();
    PIW_DECLSPEC_FUNC(d2d_nb_t) grist_filter();
}

#endif
