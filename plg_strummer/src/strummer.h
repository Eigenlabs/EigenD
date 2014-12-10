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

#ifndef __STRM_STRUMMER__
#define __STRM_STRUMMER__

#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>
#include <piw/piw_channeliser.h>

namespace strm
{
    class strummer_t;

    class strumconfig_t
    {
        public:
            class impl_t;

            strumconfig_t();
            ~strumconfig_t();

            strumconfig_t(const strumconfig_t &);
            strumconfig_t &operator=(const strumconfig_t &);

            void enable(bool);

            void set_trigger_window(unsigned);

            void set_course_pressure_scale(float);
            void set_course_roll_scale(float);
            void set_course_yaw_scale(float);
            void set_strum_breath_scale(float);
            void set_strum_pressure_scale(float);
            void set_strum_roll_scale(float);
            void set_strum_roll_pressure_mix(float);
            void set_strum_yaw_scale(float);
            void set_strum_yaw_pressure_mix(float);

            void set_strum_note_end(bool);
            void set_poly_courses_enable(bool);

            void set_course_mute_threshold(float);
            void set_course_mute_interval(unsigned);
            void set_course_mute_enable(bool);
            void set_strum_mute_threshold(float);
            void set_strum_mute_interval(unsigned);
            void set_strum_mute_enable(bool);
            void set_pulloff_threshold(float);
            void set_pulloff_interval(unsigned);
            void set_pulloff_enable(bool);

            void open_course_enable(bool);
            void open_course_pressure_default(float);
            void open_course_roll_default(float);
            void open_course_yaw_default(float);

            void clear_breath_courses();
            void add_breath_course(int);
            void remove_breath_course(int);

            void clear_key_courses();
            void add_key_course(int,int,int);
            void clear_key_course(int);

            void clear_open_courses_info();
            void set_open_course_info(int,int,int,int);
            void set_open_course_key(int,int);
            void set_open_course_physical(int,int,int);
            void clear_open_course_info(int);

            std::string encode_courses() const;

        private:
            friend class strummer_t;

            impl_t *impl_;
    };

    class strummer_t
    {
        public:
            class impl_t;

            strummer_t(const piw::cookie_t &, piw::clockdomain_ctl_t *);
            ~strummer_t();
            piw::cookie_t cookie();
            void set_strumconfig(const strumconfig_t &c);

        private:
            impl_t *impl_;
    };

    class cstrummer_t
    {
        public:
            class strummer_delegate_t: public piw::channeliser_t::delegate_t
            {
                public:
                    strummer_delegate_t(piw::clockdomain_ctl_t *cd): clock_domain_(cd), strumconfig_valid_(false)
                    {
                    }

                    piw::channeliser_t::channel_t *create_channel(const piw::cookie_t &output_cookie)
                    {
                        strummer_channel_t *s = new strummer_channel_t(output_cookie, clock_domain_);

                        if(strumconfig_valid_)
                        {
                            s->set_strumconfig(strumconfig_);
                        }

                        return s;
                    }

                    void set_strumconfig(const strumconfig_t &c)
                    {
                        strumconfig_ = c;
                        strumconfig_valid_ = true;
                    }

                private:
                    strumconfig_t strumconfig_;
                    piw::clockdomain_ctl_t *clock_domain_;
                    bool strumconfig_valid_;
            };

            class strummer_channel_t: public strummer_t, public piw::channeliser_t::channel_t
            {
                public:
                    strummer_channel_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *cd): strummer_t(o,cd)
                    {
                    }

                    void visited(const void *arg)
                    {
                        set_strumconfig(*(strumconfig_t *)arg);
                    }

                    piw::cookie_t cookie()
                    {
                        return strummer_t::cookie();
                    }
            };

        public:
            cstrummer_t(const piw::cookie_t &output, piw::clockdomain_ctl_t *cd): delegate_(cd), channeliser_(cd, &delegate_, output)
            {
            }

            piw::cookie_t cookie()
            {
                return channeliser_.cookie();
            }

            void set_strumconfig(const strumconfig_t &c)
            {
                delegate_.set_strumconfig(c);
                channeliser_.visit_channels(&c);
            }

        private:
            strummer_delegate_t delegate_;
            piw::channeliser_t channeliser_;
    };
};

#endif
