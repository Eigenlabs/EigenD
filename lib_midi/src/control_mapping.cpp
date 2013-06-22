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

#include <piw/piw_state.h>
#include <piw/piw_tsd.h>

#include <lib_midi/control_mapping.h>
#include <lib_midi/midi_gm.h>

namespace midi
{
    /*
     * decimation_handler_t
     */

    class decimation_handler_t
    {
        public:
            decimation_handler_t() {};
            virtual ~decimation_handler_t() {};

            virtual bool valid_for_processing(const mapping_data_t &data, const piw::data_nb_t &id, unsigned long long current_time) = 0;
            virtual void done_processing(const piw::data_nb_t &id) = 0;
            virtual decimation_handler_t* new_instance() = 0;
    };
}

namespace
{
    /*
     * global_decimation_handler_t
     */

    class global_decimation_handler_t: public midi::decimation_handler_t
    {
        public:
            global_decimation_handler_t() : last_processed_(0) {};

            bool valid_for_processing(const midi::mapping_data_t &data, const piw::data_nb_t &id, unsigned long long current_time)
            {
                if(!data.decimation_)
                {
                    return true;
                }

                if(last_processed_ + (data.decimation_*1000) > current_time)
                {
                    return false;
                }

                last_processed_ = current_time;
                return true;
            }

            void done_processing(const piw::data_nb_t &id)
            {
            }

            decimation_handler_t* new_instance()
            {
                return new global_decimation_handler_t();
            }

        private:
            unsigned long long last_processed_;
    };

    /*
     * perid_decimation_handler_t
     */

    typedef pic::lckmap_t<piw::data_nb_t,unsigned long long>::nbtype nb_lastprocessed_map_t;

    class perid_decimation_handler_t: public midi::decimation_handler_t
    {
        public:
            bool valid_for_processing(const midi::mapping_data_t &data, const piw::data_nb_t &id, unsigned long long current_time)
            {
                if(!data.decimation_)
                {
                    return true;
                }

                nb_lastprocessed_map_t::iterator il;
                il = last_processed_.find(id);
                if(il == last_processed_.end())
                {
                    last_processed_.insert(std::make_pair(id, current_time));
                }
                else
                {
                    unsigned long long last = il->second;
                    if(last + (data.decimation_*1000) > current_time)
                    {
                        return false;
                    }
                    
                    il->second = current_time;
                }

                return true;
            }

            void done_processing(const piw::data_nb_t &id)
            {
                last_processed_.erase(id);
            }

            decimation_handler_t* new_instance()
            {
                return new perid_decimation_handler_t();
            }

        private:
            nb_lastprocessed_map_t last_processed_;
    };
}

namespace midi
{
    /*
     * mapping_data_t
     */

    float mapping_data_t::calculate(float norm_data) const
    {
        float d = norm_data;
        if (CURVE_CUBIC == curve_)
        {
            if (d < 0) d = -d*d;
            else d = d*d;
        }
        else if (CURVE_QUADRATIC == curve_)
        {
            d = d*d*d;
        }
        else if (CURVE_STEP == curve_)
        {
            if (d <= -0.5f) d = -1.0f;
            else if (d >= 0.5f) d = 1.0f;
            else d = 0.0f;
        }
        float v = d*scale_;
        return v*lo_*(v<0) + base_ + v*hi_*(v>0);
    }


    /*
     * mapping_wrapper_t
     */

    mapping_wrapper_t::mapping_wrapper_t(bool decimate_per_id, float scale, float lo, float base, float hi, bool origin_return,
            float decimation, unsigned scope, unsigned channel, unsigned resolution, int secondary,
            unsigned curve): mapping_data_t(scale, lo, base, hi, origin_return, decimation, scope, channel,
                resolution, secondary, curve),
            decimation_handler_(decimate_per_id ? (decimation_handler_t *)new perid_decimation_handler_t() : (decimation_handler_t *)new global_decimation_handler_t())
    {
    };

    mapping_wrapper_t::mapping_wrapper_t(const mapping_wrapper_t &o): mapping_data_t(o), decimation_handler_(o.decimation_handler_->new_instance())
    {
    };

    mapping_wrapper_t::~mapping_wrapper_t()
    {
        delete decimation_handler_;
    };

    bool mapping_wrapper_t::valid_for_processing(const piw::data_nb_t &id, unsigned long long current_time)
    {
        return decimation_handler_->valid_for_processing(*this, id, current_time);
    };

    void mapping_wrapper_t::done_processing(const piw::data_nb_t &id)
    {
        decimation_handler_->done_processing(id);
    };


    /*
     * global_settings_t
     */

    global_settings_t global_settings_t::clone_with_midi_channel(unsigned channel)
    {
        return global_settings_t(channel, minimum_midi_channel_, maximum_midi_channel_, minimum_decimation_, send_notes_, send_pitchbend_, send_hires_velocity_, pitchbend_semitones_up_, pitchbend_semitones_down_);
    }

    global_settings_t global_settings_t::clone_with_minimum_midi_channel(unsigned min)
    {
        return global_settings_t(midi_channel_, min, maximum_midi_channel_, minimum_decimation_, send_notes_, send_pitchbend_, send_hires_velocity_, pitchbend_semitones_up_, pitchbend_semitones_down_);
    }

    global_settings_t global_settings_t::clone_with_maximum_midi_channel(unsigned max)
    {
        return global_settings_t(midi_channel_, minimum_midi_channel_, max, minimum_decimation_, send_notes_, send_pitchbend_, send_hires_velocity_, pitchbend_semitones_up_, pitchbend_semitones_down_);
    }

    global_settings_t global_settings_t::clone_with_minimum_decimation(float dec)
    {
        return global_settings_t(midi_channel_, minimum_midi_channel_, maximum_midi_channel_, dec, send_notes_, send_pitchbend_, send_hires_velocity_, pitchbend_semitones_up_, pitchbend_semitones_down_);
    }

    global_settings_t global_settings_t::clone_with_send_notes(bool notes)
    {
        return global_settings_t(midi_channel_, minimum_midi_channel_, maximum_midi_channel_, minimum_decimation_, notes, send_pitchbend_, send_hires_velocity_, pitchbend_semitones_up_, pitchbend_semitones_down_);
    }

    global_settings_t global_settings_t::clone_with_send_pitchbend(bool pb)
    {
        return global_settings_t(midi_channel_, minimum_midi_channel_, maximum_midi_channel_, minimum_decimation_, send_notes_, pb, send_hires_velocity_, pitchbend_semitones_up_, pitchbend_semitones_down_);
    }

    global_settings_t global_settings_t::clone_with_send_hires_velocity(bool hivel)
    {
        return global_settings_t(midi_channel_, minimum_midi_channel_, maximum_midi_channel_, minimum_decimation_, send_notes_, send_pitchbend_, hivel, pitchbend_semitones_up_, pitchbend_semitones_down_);
    }

    global_settings_t global_settings_t::clone_with_pitchbend_semitones_up(float pbup)
    {
        return global_settings_t(midi_channel_, minimum_midi_channel_, maximum_midi_channel_, minimum_decimation_, send_notes_, send_pitchbend_, send_hires_velocity_, pbup, pitchbend_semitones_down_);
    }

    global_settings_t global_settings_t::clone_with_pitchbend_semitones_down(float pbdown)
    {
        return global_settings_t(midi_channel_, minimum_midi_channel_, maximum_midi_channel_, minimum_decimation_, send_notes_, send_pitchbend_, send_hires_velocity_, pitchbend_semitones_up_, pbdown);
    }

    bool global_settings_t::operator==(const global_settings_t &o) const
    {
        return o.midi_channel_ == midi_channel_ && 
            o.minimum_midi_channel_ == minimum_midi_channel_ && 
            o.maximum_midi_channel_ == maximum_midi_channel_ &&
            o.minimum_decimation_ == minimum_decimation_ &&
            o.send_notes_ == send_notes_ && 
            o.send_pitchbend_ == send_pitchbend_ && 
            o.send_hires_velocity_ == send_hires_velocity_ &&
            o.pitchbend_semitones_up_ == pitchbend_semitones_up_ && 
            o.pitchbend_semitones_down_ == pitchbend_semitones_down_;
    }


    /*
     * mapping_info_t
     */

    mapping_info_t mapping_info_t::clone_with_enabled(bool enabled)
    {
        return mapping_info_t(oparam_, enabled, scale_, lo_, base_, hi_, origin_return_, decimation_, scope_, channel_, resolution_, secondary_cc_, curve_);
    }

    mapping_info_t mapping_info_t::clone_with_scale(float scale)
    {
        return mapping_info_t(oparam_, enabled_, scale, lo_, base_, hi_, origin_return_, decimation_, scope_, channel_, resolution_, secondary_cc_, curve_);
    }

    mapping_info_t mapping_info_t::clone_with_lo(float lo)
    {
        return mapping_info_t(oparam_, enabled_, scale_, lo, base_, hi_, origin_return_, decimation_, scope_, channel_, resolution_, secondary_cc_, curve_);
    }

    mapping_info_t mapping_info_t::clone_with_base(float base)
    {
        return mapping_info_t(oparam_, enabled_, scale_, lo_, base, hi_, origin_return_, decimation_, scope_, channel_, resolution_, secondary_cc_, curve_);
    }

    mapping_info_t mapping_info_t::clone_with_hi(float hi)
    {
        return mapping_info_t(oparam_, enabled_, scale_, lo_, base_, hi, origin_return_, decimation_, scope_, channel_, resolution_, secondary_cc_, curve_);
    }

    mapping_info_t mapping_info_t::clone_with_origin_return(bool origin_return)
    {
        return mapping_info_t(oparam_, enabled_, scale_, lo_, base_, hi_, origin_return, decimation_, scope_, channel_, resolution_, secondary_cc_, curve_);
    }

    mapping_info_t mapping_info_t::clone_with_decimation(float decimation)
    {
        return mapping_info_t(oparam_, enabled_, scale_, lo_, base_, hi_, origin_return_, decimation, scope_, channel_, resolution_, secondary_cc_, curve_);
    }

    mapping_info_t mapping_info_t::clone_with_scope(unsigned scope)
    {
        return mapping_info_t(oparam_, enabled_, scale_, lo_, base_, hi_, origin_return_, decimation_, scope, channel_, resolution_, secondary_cc_, curve_);
    }

    mapping_info_t mapping_info_t::clone_with_channelscope(unsigned channel)
    {
        return mapping_info_t(oparam_, enabled_, scale_, lo_, base_, hi_, origin_return_, decimation_, CHANNEL_SCOPE, channel, resolution_, secondary_cc_, curve_);
    }

    mapping_info_t mapping_info_t::clone_with_resolution(unsigned resolution)
    {
        return mapping_info_t(oparam_, enabled_, scale_, lo_, base_, hi_, origin_return_, decimation_, scope_, channel_, resolution, secondary_cc_, curve_);
    }

    mapping_info_t mapping_info_t::clone_with_secondarycc(int secondary)
    {
        return mapping_info_t(oparam_, enabled_, scale_, lo_, base_, hi_, origin_return_, decimation_, scope_, channel_, resolution_, secondary, curve_);
    }

    mapping_info_t mapping_info_t::clone_with_curve(unsigned curve)
    {
        return mapping_info_t(oparam_, enabled_, scale_, lo_, base_, hi_, origin_return_, decimation_, scope_, channel_, resolution_, secondary_cc_, curve);
    }


    /*
     * controllers_mapping_t
     */

    unsigned controllers_mapping_t::get_serial()
    {
        pic::flipflop_t<mapping_t>::guard_t mg(mapping_);
        return mg.value().serial_;
    }

    void controllers_mapping_t::set_mapping(const std::string &mapping)
    {
        piw::term_t t(piw::parse_state_term(mapping)); 
        mapping_.alternate().map_params_.clear();
        mapping_.alternate().map_midi_.clear();
        for(unsigned i=0; i<t.arity(); ++i)
        {
            piw::term_t t2(t.arg(i));

            if(0==strcmp("m",t2.pred()) || 0==strcmp("c",t2.pred()))
            {
                bool au_mapping = (0==strcmp("m",t2.pred()));
                bool midi_mapping = (0==strcmp("c",t2.pred()));

                long iparam = -1;
                int oparam = -1;
                float scale = 0.f;
                float lo = 1.f;
                float base = 0.f;
                float hi = 1.f;
                bool origin_return = false;
                float decimation = 0.f;
                bool enabled = true;
                unsigned scope = GLOBAL_SCOPE;
                unsigned channel = 0;
                unsigned resolution = BITS_7;
                int secondary_cc = -1;
                unsigned curve = CURVE_LINEAR;

                if(au_mapping)
                {
                    origin_return = true;
                }
                if(midi_mapping)
                {
                    decimation = 10.f;
                }

                if(t2.arity()>=3 && 
                    t2.arg(0).value().type()==BCTVTYPE_INT && 
                    t2.arg(1).value().type()==BCTVTYPE_INT &&
                    t2.arg(2).value().type()==BCTVTYPE_FLOAT)
                {
                    iparam = t2.arg(0).value().as_long();
                    oparam = t2.arg(1).value().as_long();
                    scale = t2.arg(2).value().as_float();

                    if(t2.arity()>=13 && 
                        t2.arg(3).value().type()==BCTVTYPE_FLOAT && 
                        t2.arg(4).value().type()==BCTVTYPE_FLOAT && 
                        t2.arg(5).value().type()==BCTVTYPE_FLOAT && 
                        t2.arg(6).value().type()==BCTVTYPE_BOOL && 
                        t2.arg(7).value().type()==BCTVTYPE_FLOAT && 
                        t2.arg(8).value().type()==BCTVTYPE_BOOL && 
                        t2.arg(9).value().type()==BCTVTYPE_INT && 
                        t2.arg(10).value().type()==BCTVTYPE_INT && 
                        t2.arg(11).value().type()==BCTVTYPE_INT && 
                        t2.arg(12).value().type()==BCTVTYPE_INT)
                    {
                        lo = t2.arg(3).value().as_float();
                        base = t2.arg(4).value().as_float();
                        hi = t2.arg(5).value().as_float();
                        origin_return = t2.arg(6).value().as_bool();
                        decimation = t2.arg(7).value().as_float();
                        enabled = t2.arg(8).value().as_bool();
                        scope = t2.arg(9).value().as_long();
                        channel = t2.arg(10).value().as_long();
                        resolution = t2.arg(11).value().as_long();
                        secondary_cc = t2.arg(12).value().as_long();

                        if(t2.arity()>=14 && 
                            t2.arg(13).value().type()==BCTVTYPE_INT)
                        {
                            curve = t2.arg(13).value().as_long();
                        }
                    }
                }

                if(iparam>=0)
                {
                    if(au_mapping)
                    {
                        mapping_.alternate().map_params_.insert(std::make_pair(iparam, mapping_info_t(oparam,enabled,scale,lo,base,hi,origin_return,decimation,scope,channel,resolution,secondary_cc,curve)));
                    }
                    if(midi_mapping)
                    {
                        mapping_.alternate().map_midi_.insert(std::make_pair(iparam, mapping_info_t(oparam,enabled,scale,lo,base,hi,origin_return,decimation,scope,channel,resolution,secondary_cc,curve)));
                    }
                }
            }
        }

        mapping_.alternate().serial_++;
        mapping_.exchange();
        listener_.mapping_changed(mapping);
        listener_.settings_changed();
    }

    std::string controllers_mapping_t::get_mapping()
    {
        pic::flipflop_t<mapping_t>::guard_t mg(mapping_);
        piw::term_t t("mapping",mg.value().map_params_.size()+mg.value().map_midi_.size());

        control_map_t::const_iterator mi, me;
        unsigned i = 0;

        mi = mg.value().map_params_.begin();
        me = mg.value().map_params_.end();
        for(; mi!=me; ++mi,++i)
        {
            piw::term_t t2("m",14);
            t2.set_arg(0,piw::makelong(mi->first));
            t2.set_arg(1,piw::makelong(mi->second.oparam_));
            t2.set_arg(2,piw::makefloat(mi->second.scale_));
            t2.set_arg(3,piw::makefloat(mi->second.lo_));
            t2.set_arg(4,piw::makefloat(mi->second.base_));
            t2.set_arg(5,piw::makefloat(mi->second.hi_));
            t2.set_arg(6,piw::makebool(mi->second.origin_return_));
            t2.set_arg(7,piw::makefloat(mi->second.decimation_));
            t2.set_arg(8,piw::makebool(mi->second.enabled_));
            t2.set_arg(9,piw::makelong(mi->second.scope_));
            t2.set_arg(10,piw::makelong(mi->second.channel_));
            t2.set_arg(11,piw::makelong(mi->second.resolution_));
            t2.set_arg(12,piw::makelong(mi->second.secondary_cc_));
            t2.set_arg(13,piw::makelong(mi->second.curve_));
            t.set_arg(i,t2);
        }

        mi = mg.value().map_midi_.begin();
        me = mg.value().map_midi_.end();
        for(; mi!=me; ++mi,++i)
        {
            piw::term_t t2("c",14);
            t2.set_arg(0,piw::makelong(mi->first));
            t2.set_arg(1,piw::makelong(mi->second.oparam_));
            t2.set_arg(2,piw::makefloat(mi->second.scale_));
            t2.set_arg(3,piw::makefloat(mi->second.lo_));
            t2.set_arg(4,piw::makefloat(mi->second.base_));
            t2.set_arg(5,piw::makefloat(mi->second.hi_));
            t2.set_arg(6,piw::makebool(mi->second.origin_return_));
            t2.set_arg(7,piw::makefloat(mi->second.decimation_));
            t2.set_arg(8,piw::makebool(mi->second.enabled_));
            t2.set_arg(9,piw::makelong(mi->second.scope_));
            t2.set_arg(10,piw::makelong(mi->second.channel_));
            t2.set_arg(11,piw::makelong(mi->second.resolution_));
            t2.set_arg(12,piw::makelong(mi->second.secondary_cc_));
            t2.set_arg(13,piw::makelong(mi->second.curve_));
            t.set_arg(i,t2);
        }

        return t.render();
    }

    void controllers_mapping_t::unmap_span_overlaps(unsigned iparam, int oparam)
    {
        control_map_t &m = mapping_.alternate().map_params_;
        global_settings_t &s = mapping_.alternate().settings_;

        int poly_range = s.maximum_midi_channel_ - s.minimum_midi_channel_;
        std::pair<control_map_t::iterator,control_map_t::iterator> i(m.equal_range(iparam));
        while(i.first!=i.second && i.first!=m.end())
        {
            if(i.first->first==iparam &&
               (i.first->second.oparam_>oparam &&
                i.first->second.oparam_<=oparam+poly_range))
            {
                control_map_t::iterator to_erase = i.first;
                i.first++;
                m.erase(to_erase);
            }
            else
            {
                i.first++;
            }
        }
        mapping_.alternate().serial_++;
        mapping_.exchange();
    }

    void controllers_mapping_t::map_param(unsigned iparam, mapping_info_t &info)
    {
        global_settings_t &s = mapping_.alternate().settings_;

        // ensure that this mapping is not overlapping with an
        // existing per-note span and the midi channel scope
        if(info.oparam_ > 0 && 0 == s.midi_channel_)
        {
            control_map_t &m = mapping_.alternate().map_params_;
            int poly_range = s.maximum_midi_channel_ - s.minimum_midi_channel_;
            for(int o = info.oparam_-1; o >= 0 && o >= info.oparam_-poly_range; --o)
            {
                mapping_info_t poly_info = get_info(m,iparam,o);
                if(poly_info.is_valid() && PERNOTE_SCOPE == poly_info.scope_)
                {
                    return;
                }
            }
        }
         
        // ensure that there are no param mappings that overlap
        // with possible per-note spans and the midi channel scope
        if(PERNOTE_SCOPE == info.scope_ && 0 == s.midi_channel_)
        {
            unmap_span_overlaps(iparam, info.oparam_);
        }

        map(mapping_.alternate().map_params_,iparam,info);
    }

    void controllers_mapping_t::map_midi(unsigned iparam, mapping_info_t &info)
    {
        map(mapping_.alternate().map_midi_,iparam,info);
    }

    void controllers_mapping_t::map(control_map_t &m, unsigned iparam, mapping_info_t info)
    {
        std::pair<control_map_t::iterator,control_map_t::iterator> i(m.equal_range(iparam));
        while(i.first!=i.second && i.first!=m.end())
        {
            if(i.first->first==iparam &&
               i.first->second.oparam_==info.oparam_)
            {
                control_map_t::iterator to_erase = i.first;
                i.first++;
                m.erase(to_erase);
            }
            else
            {
                i.first++;
            }
        }

        m.insert(
            std::make_pair(iparam,info));
        mapping_.alternate().serial_++;
        mapping_.exchange();
        listener_.mapping_changed(get_mapping());
        listener_.parameter_changed(iparam);
    }

    void controllers_mapping_t::unmap_param(unsigned iparam, unsigned short oparam)
    {
        unmap(mapping_.alternate().map_params_,iparam,oparam);
    }

    void controllers_mapping_t::unmap_midi(unsigned iparam, unsigned short oparam)
    {
        unmap(mapping_.alternate().map_midi_,iparam,oparam);
    }

    void controllers_mapping_t::unmap(control_map_t &m, unsigned iparam, unsigned short oparam)
    {
        std::pair<control_map_t::iterator,control_map_t::iterator> i(m.equal_range(iparam));
        
        while(i.first!=i.second && i.first!=m.end())
        {
            if(i.first->first==iparam &&
               i.first->second.oparam_==oparam)
            {
                m.erase(i.first);
                mapping_.alternate().serial_++;
                mapping_.exchange();
                listener_.mapping_changed(get_mapping());
                listener_.parameter_changed(iparam);
                return;
            }
            else
            {
                i.first++;
            }
        }
    }

    bool controllers_mapping_t::is_mapped_param(unsigned iparam, unsigned short oparam)
    {
        pic::flipflop_t<mapping_t>::guard_t mg(mapping_);
        return is_mapped(mg.value().map_params_,iparam,oparam);
    }

    bool controllers_mapping_t::is_mapped_midi(unsigned iparam, unsigned short oparam)
    {
        pic::flipflop_t<mapping_t>::guard_t mg(mapping_);
        return is_mapped(mg.value().map_midi_,iparam,oparam);
    }

    bool controllers_mapping_t::is_mapped(const control_map_t &m, unsigned iparam, unsigned short oparam)
    {
        std::pair<control_map_t::const_iterator,control_map_t::const_iterator> i(m.equal_range(iparam));
        
        for(; i.first!=i.second; ++i.first)
        {
            if(i.first->first==iparam && i.first->second.oparam_==oparam)
            {
                return true;
            }
        }
        return false;
    }

    mapping_info_t controllers_mapping_t::get_info_param(unsigned iparam, unsigned short oparam)
    {
        pic::flipflop_t<mapping_t>::guard_t mg(mapping_);
        return get_info(mg.value().map_params_,iparam,oparam);
    }

    mapping_info_t controllers_mapping_t::get_info_midi(unsigned iparam, unsigned short oparam)
    {
        pic::flipflop_t<mapping_t>::guard_t mg(mapping_);
        return get_info(mg.value().map_midi_,iparam,oparam);
    }

    mapping_info_t controllers_mapping_t::get_info(const control_map_t &m, unsigned iparam, unsigned short oparam)
    {
        std::pair<control_map_t::const_iterator,control_map_t::const_iterator> i(m.equal_range(iparam));
        
        for(; i.first!=i.second; ++i.first)
        {
            if(i.first->first==iparam && i.first->second.oparam_==oparam)
            {
                return i.first->second;
            }
        }
        return mapping_info_t();
    }

    void controllers_mapping_t::clear_params()
    {
        mapping_.alternate().map_params_.clear();
        mapping_.alternate().serial_++;
        mapping_.exchange();
        listener_.mapping_changed(get_mapping());
        listener_.parameter_changed(1);
    }

    void controllers_mapping_t::clear_midi_cc()
    {
        control_map_t &m = mapping_.alternate().map_midi_;
        control_map_t::iterator i(m.begin());
        
        while(i!=m.end())
        {
            if(i->second.oparam_<MIDI_CC_MAX)
            {
                control_map_t::iterator to_erase = i;
                i++;
                m.erase(to_erase);
            }
            else
            {
                i++;
            }
        }
        mapping_.alternate().serial_++;
        mapping_.exchange();
        listener_.mapping_changed(get_mapping());
        listener_.parameter_changed(1);
    }

    void controllers_mapping_t::clear_midi_behaviour()
    {
        control_map_t &m = mapping_.alternate().map_midi_;
        control_map_t::iterator i(m.begin());
        
        while(i!=m.end())
        {
            if(i->second.oparam_>=MIDI_CC_MAX)
            {
                control_map_t::iterator to_erase = i;
                i++;
                m.erase(to_erase);
            }
            else
            {
                i++;
            }
        }
        mapping_.alternate().serial_++;
        mapping_.exchange();
        listener_.mapping_changed(get_mapping());
        listener_.parameter_changed(1);
    }

    global_settings_t controllers_mapping_t::get_settings()
    {
        pic::flipflop_t<mapping_t>::guard_t mg(mapping_);
        return mg.value().settings_;
    }

    void controllers_mapping_t::change_settings(global_settings_t settings)
    {
        if (mapping_.alternate().settings_ == settings)
        {
            return;
        }

        mapping_.alternate().settings_ = settings;
        mapping_.alternate().serial_++;
        mapping_.exchange();
        listener_.mapping_changed(get_mapping());
        settings_changed();
    }

    void controllers_mapping_t::settings_changed()
    {
        global_settings_t &s = mapping_.alternate().settings_;

        // ensure that there are no param mappings that overlap
        // with possible per-note spans and the midi channel scope
        if(0 == s.midi_channel_)
        {
            std::map<unsigned, int> spans;

            control_map_t &m = mapping_.alternate().map_params_;
            control_map_t::iterator it;
            for(it=m.begin(); it!=m.end(); it++)
            {
                if(it->second.oparam_ >=0 && PERNOTE_SCOPE == it->second.scope_)
                {
                    spans.insert(std::make_pair(it->first, it->second.oparam_));
                }
            }

            std::map<unsigned, int>::iterator s_it;
            for(s_it = spans.begin(); s_it != spans.end(); s_it++)
            {
                unmap_span_overlaps(s_it->first, s_it->second);
            }

            mapping_.alternate().serial_++;
            mapping_.exchange();
            listener_.mapping_changed(get_mapping());
        }

        listener_.settings_changed();
        listener_.parameter_changed(1);
    }

    controllers_map_range_t controllers_mapping_t::param_mappings(unsigned name)
    {
       PIC_ASSERT(acquired_);
       return acquired_->map_params_.equal_range(name);
    }

    controllers_map_range_t controllers_mapping_t::midi_mappings(unsigned name)
    {
       PIC_ASSERT(acquired_);
       return acquired_->map_midi_.equal_range(name);
    }

    void controllers_mapping_t::refresh_mappings(control_mapping_t &mapping)
    {
        pic::untyped_guard_t mg(this);

        unsigned current_serial = get_serial();
        if(current_serial != mapping.mapping_serial_)
        {
            mapping.mapping_serial_ = current_serial;

            refresh_params_(mapping);
            refresh_midi_(mapping);
        }
    }

    void controllers_mapping_t::refresh_params(control_mapping_t &mapping)
    {
        pic::untyped_guard_t mg(this);

        unsigned current_serial = get_serial();
        if(current_serial != mapping.mapping_serial_)
        {
            mapping.mapping_serial_ = current_serial;

            refresh_params_(mapping);
        }
    }

    void controllers_mapping_t::refresh_origins(control_mapping_t &mapping, pic::i2f_t param_origin)
    {
        pic::untyped_guard_t mg(this);

        unsigned long long current_time = piw::tsd_time();
        if(current_time-mapping.origins_update_<200000)
        {
            return;
        }

        mapping.origins_update_ = current_time;

        controllers_map_range_t ip(param_mappings(mapping.name_));

        mapping.origins_.clear();

        for(; ip.first!=ip.second; ++ip.first)
        {
            int oparam = ip.first->second.oparam_;
            mapping.origins_.insert(std::make_pair(oparam, param_origin(oparam)));
        }

    }

    void controllers_mapping_t::refresh_midi(control_mapping_t &mapping)
    {
        pic::untyped_guard_t mg(this);

        unsigned current_serial = get_serial();
        if(current_serial != mapping.mapping_serial_)
        {
            mapping.mapping_serial_ = current_serial;

            refresh_midi_(mapping);
        }
    }

    void controllers_mapping_t::refresh_params_(control_mapping_t &mapping)
    {
        PIC_ASSERT(acquired_);

        controllers_map_range_t ip(param_mappings(mapping.name_));

        mapping.params_.clear();

        for(; ip.first!=ip.second; ++ip.first)
        {
            if(ip.first->second.is_valid() && ip.first->second.enabled_)
            {
                mapping.params_.insert(std::make_pair(ip.first->second.oparam_, 
                    mapping_wrapper_t(false, ip.first->second.scale_, ip.first->second.lo_, ip.first->second.base_,
                        ip.first->second.hi_, ip.first->second.origin_return_, std::max(acquired_->settings_.minimum_decimation_,ip.first->second.decimation_),
                        ip.first->second.scope_, ip.first->second.channel_, ip.first->second.resolution_,
                        ip.first->second.secondary_cc_, ip.first->second.curve_)));
            }
        }
    }

    void controllers_mapping_t::refresh_midi_(control_mapping_t &mapping)
    {
        PIC_ASSERT(acquired_);

        controllers_map_range_t ic(midi_mappings(mapping.name_));

        mapping.midi_.clear();

        for(; ic.first!=ic.second; ++ic.first)
        {
            if(ic.first->second.is_valid() && ic.first->second.enabled_)
            {
                bool decimate_per_id = false;
                unsigned oparam = ic.first->second.oparam_;
                if(oparam == MIDI_CC_MAX + POLY_AFTERTOUCH)
                {
                    decimate_per_id = true;
                }

                mapping.midi_.insert(std::make_pair(oparam,
                    mapping_wrapper_t(decimate_per_id, ic.first->second.scale_, ic.first->second.lo_, ic.first->second.base_,
                        ic.first->second.hi_, ic.first->second.origin_return_, std::max(acquired_->settings_.minimum_decimation_,ic.first->second.decimation_),
                        ic.first->second.scope_, ic.first->second.channel_, ic.first->second.resolution_,
                        ic.first->second.secondary_cc_, ic.first->second.curve_)));
            }
        }
    }

    void controllers_mapping_t::acquire()
    {
        const mapping_t* acquired=mapping_.acquire();
        PIC_ASSERT(!acquired_);
        acquired_=acquired;
    }

    void controllers_mapping_t::release()
    {
        PIC_ASSERT(acquired_);
        const mapping_t* acquired=acquired_;
        acquired_=0;
        mapping_.release(acquired);
    }
    
} // namespace midi
