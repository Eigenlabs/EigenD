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

#ifndef __FNG_FINGER__
#define __FNG_FINGER__

#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>
#include <piw/piw_channeliser.h>
#include <picross/pic_stl.h>
#include <bitset>
#include <algorithm>

//Maximum number of keys and courses.
#define MAX_KEYS 9
#define MAX_COURSES 2

namespace fng
{
    class fingering_t
    {
        public:
            class impl_t;

        public:
            typedef pic::lckvector_t<std::pair<unsigned,unsigned> >::nbtype keylist_t;
            typedef std::pair<keylist_t,float> mapping_t;

            typedef std::bitset<MAX_KEYS*MAX_COURSES> f_pattern_t;                                    //the binary bitmap of the fingering
            typedef std::pair<f_pattern_t, float> f_fingering_t;                                      //pair of the binary fingering bitmap and other data
            typedef pic::lckvector_t<f_fingering_t *>::nbtype f_fingering_pointers_t;                 //vector of pointers to other fingerings  
            typedef std::pair<f_fingering_t, f_fingering_pointers_t> f_fingerings_t;                  //pair of the fingering and vector of subfingerings  
            typedef pic::lckvector_t<f_fingerings_t>::nbtype table_t;                                 //vector of individual fingerings

            typedef std::pair<float, float> range_t;                                                  //min and max values of the pressure 

        public:
            fingering_t() {}

            fingering_t(const fingering_t &f): table_(f.table_) 
            { 
                table_ = f.table_; 
                modifiers_ = f.modifiers_; 
                additions_ = f.additions_; 
                polyphonies_ = f.polyphonies_; 
                open_is_set_ = f.open_is_set_;
                range_ = f.range_;
                breath_threshold_ = f.breath_threshold_;
                needed_poly_ = f.needed_poly_;
                evaluate_table();
            }

            fingering_t &operator=(const fingering_t &f) 
            {
                table_ = f.table_; 
                modifiers_ = f.modifiers_; 
                additions_ = f.additions_; 
                polyphonies_ = f.polyphonies_; 
                open_is_set_ = f.open_is_set_;
                range_ = f.range_;
                breath_threshold_ = f.breath_threshold_;
                needed_poly_ = f.needed_poly_;
                evaluate_table();
                return *this; 
            }

            void clear_table()
            {
                table_ = table_t();
                modifiers_ = table_t(); 
                additions_ = table_t(); 
                polyphonies_ = table_t();
                open_is_set_=false;
                needed_poly_ = 1;
            }

            static bool _compare_fingerings_table(const f_fingerings_t &a, const f_fingerings_t &b)
            {
                return a.first.first.count() > b.first.first.count();
            }

            static bool _compare_fingerings(const f_fingering_t *a, const f_fingering_t *b)
            {
                return a->first.count() > b->first.count();
            }

            const table_t &get_table() const 
            { 
                return table_; 
            }

            const table_t &get_modifiers() const 
            { 
                return modifiers_; 
            }

            const table_t &get_additions() const 
            { 
                return additions_; 
            }

            const table_t &get_polyphonies() const 
            { 
                return polyphonies_; 
            }

            const range_t &get_range() const
            {
                return range_;
            }

            void set_range(float min, float max)
            {
                if(min == 0)
                { 
                    min = 0.0001;
                }
                range_.first = min;
                range_.second = 1/max;
            }

            const float &get_breath_threshold() const
            {
                return breath_threshold_;
            }

            void set_breath_threshold(float b)
            {
                breath_threshold_ = b;
            }

            void set_open()
            {
                open_is_set_=true;
            }

            const bool &open_is_set() const
            {
                return open_is_set_;
            }

            void evaluate_table()
            {
                stable_sort(table_.begin(), table_.end(), _compare_fingerings_table);
                for(unsigned i=0;i<table_.size();i++)
                {
                    table_[i].second = f_fingering_pointers_t();
                    for(unsigned j=0;j<table_.size();j++)
                    {
                        if((table_[i].first.first & table_[j].first.first) == table_[j].first.first)
                        {
                            table_[i].second.push_back(&table_[j].first);
                        }
                    }
                    stable_sort(table_[i].second.begin(), table_[i].second.end(), _compare_fingerings);
                }


                stable_sort(modifiers_.begin(), modifiers_.end(), _compare_fingerings_table);
                for(unsigned i=0;i<modifiers_.size();i++)
                {
                    modifiers_[i].second = f_fingering_pointers_t();
                    for(unsigned j=0;j<modifiers_.size();j++)
                    {
                        if((modifiers_[i].first.first & modifiers_[j].first.first) == modifiers_[j].first.first)
                        {
                            modifiers_[i].second.push_back(&modifiers_[j].first);
                        }
                    }
                    stable_sort(modifiers_[i].second.begin(), modifiers_[i].second.end(), _compare_fingerings);
                }

                stable_sort(additions_.begin(), additions_.end(), _compare_fingerings_table);
                for(unsigned i=0;i<additions_.size();i++)
                {
                    additions_[i].second = f_fingering_pointers_t();
                    for(unsigned j=0;j<additions_.size();j++)
                    {
                        if((additions_[i].first.first & additions_[j].first.first) == additions_[j].first.first)
                        {
                            additions_[i].second.push_back(&additions_[j].first);
                        }
                    }
                    stable_sort(additions_[i].second.begin(), additions_[i].second.end(), _compare_fingerings);
                }

                stable_sort(polyphonies_.begin(), polyphonies_.end(), _compare_fingerings_table);
                for(unsigned i=0;i<polyphonies_.size();i++)
                {
                    polyphonies_[i].second = f_fingering_pointers_t();
                    for(unsigned j=0;j<polyphonies_.size();j++)
                    {
                        if((polyphonies_[i].first.first & polyphonies_[j].first.first) == polyphonies_[j].first.first)
                        {
                            polyphonies_[i].second.push_back(&polyphonies_[j].first);
                        }
                    }
                    stable_sort(polyphonies_[i].second.begin(), polyphonies_[i].second.end(), _compare_fingerings);
                }

                return;
            }

            void add_fingering_pattern(unsigned fingering_number, float output)
            {
                while(table_.size() <= fingering_number)
                {
                    table_.push_back(f_fingerings_t());
                }
                table_[fingering_number].first.second = output;
            }

            void add_key(unsigned fingering_number, unsigned course, unsigned key)
            {
                while(table_.size() <= fingering_number)
                {
                    table_.push_back(f_fingerings_t());
                }
                if((course == 0) | (key == 0))
                {
                    table_[fingering_number].first.first = 0;
                }
                else
                {
                    table_[fingering_number].first.first.set(((course-1) * MAX_KEYS) + key-1, true);
                }
            }

            void add_modifier_pattern(unsigned fingering_number, float output)
            {
                while(modifiers_.size() <= fingering_number)
                {
                    modifiers_.push_back(f_fingerings_t());
                }
                modifiers_[fingering_number].first.second = output;
            }

            void add_modifier_key(unsigned fingering_number, unsigned course, unsigned key)
            {
                while(modifiers_.size() <= fingering_number)
                {
                    modifiers_.push_back(f_fingerings_t());
                }
                modifiers_[fingering_number].first.first.set(((course-1) * MAX_KEYS) + key-1, true);
            }

            void add_addition_pattern(unsigned fingering_number, float output)
            {
                while(additions_.size() <= fingering_number)
                {
                    additions_.push_back(f_fingerings_t());
                }
                additions_[fingering_number].first.second = output;
            }

            void add_addition_key(unsigned fingering_number, unsigned course, unsigned key)
            {
                while(additions_.size() <= fingering_number)
                {
                    additions_.push_back(f_fingerings_t());
                }
                additions_[fingering_number].first.first.set(((course-1) * MAX_KEYS) + key-1, true);
            }

            void add_polyphony_pattern(unsigned fingering_number, float output)
            {
                while(polyphonies_.size() <= fingering_number)
                {
                    polyphonies_.push_back(f_fingerings_t());
                }
                polyphonies_[fingering_number].first.second = output;
            }

            void add_polyphony_key(unsigned fingering_number, unsigned course, unsigned key)
            {
                while(polyphonies_.size() <= fingering_number)
                {
                    polyphonies_.push_back(f_fingerings_t());
                }
                polyphonies_[fingering_number].first.first.set(((course-1) * MAX_KEYS) + key-1, true);
            }

            void set_poly(unsigned p)
            {
                needed_poly_ = p;            
            }

            const unsigned &get_poly() const 
            { 
                return needed_poly_; 
            }

        private:
            table_t table_;
            table_t modifiers_;
            table_t additions_;
            table_t polyphonies_;
            range_t range_;
            unsigned needed_poly_;
            float breath_threshold_;                                                                  //breath note activation threshold
            bool open_is_set_;

    };

    class finger_t
    {
        public:
            class impl_t;

        public:
            finger_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &);
            ~finger_t();
            piw::cookie_t cookie();
            void set_fingering(const fingering_t &);


        private:
            impl_t *impl_;
    };

    class cfinger_t
    {
        public:
            class finger_delegate_t: public piw::channeliser_t::delegate_t
            {
                public:
                    finger_delegate_t(piw::clockdomain_ctl_t *cd): clock_domain_(cd), fingering_valid_(false)
                    {
                    }

                    piw::channeliser_t::channel_t *create_channel(const piw::cookie_t &output_cookie)
                    {
                        finger_channel_t *f = new finger_channel_t(clock_domain_, output_cookie);

                        if(fingering_valid_)
                        {
                            f->set_fingering(fingering_);
                        }

                        return f;
                    }

                    void set_fingering(const fingering_t &f)
                    {
                        fingering_ = f;
                        fingering_valid_ = true;
                    }

                private:
                    fingering_t fingering_;
                    piw::clockdomain_ctl_t *clock_domain_;
                    bool fingering_valid_;
            };

            class finger_channel_t: public finger_t, public piw::channeliser_t::channel_t
            {
                public:
                    finger_channel_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &o): finger_t(cd,o)
                    {
                    }

                    void visited(const void *arg)
                    {
                        set_fingering(*(fingering_t *)arg);
                    }

                    piw::cookie_t cookie()
                    {
                        return finger_t::cookie();
                    }
            };

        public:
            cfinger_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &output): delegate_(cd), channeliser_(cd, &delegate_, output)
            {
            }

            piw::cookie_t cookie()
            {
                return channeliser_.cookie();
            }

            void set_fingering(const fingering_t &f)
            {
                delegate_.set_fingering(f);
                channeliser_.visit_channels(&f);
            }

        private:
            finger_delegate_t delegate_;
            piw::channeliser_t channeliser_;
    };

};

#endif
