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
#include <piw/piw_control_mapping.h>
#include <piw/piw_midi_gm.h>

namespace piw
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

            if(0==strcmp("s",t2.pred()))
            {
                float minimum_decimation = 0.f;
                bool send_notes = true;
                bool send_pitchbend = true;
                bool send_hires_velocity = false;
                if(t2.arity() >= 3 &&
                    t2.arg(0).value().type()==BCTVTYPE_FLOAT && 
                    t2.arg(1).value().type()==BCTVTYPE_BOOL && 
                    t2.arg(2).value().type()==BCTVTYPE_BOOL)
                {
                    minimum_decimation = t2.arg(0).value().as_float();
                    send_notes = t2.arg(1).value().as_bool();
                    send_pitchbend = t2.arg(2).value().as_bool();

                    if(t2.arity() >= 4 &&
                        t2.arg(3).value().type()==BCTVTYPE_BOOL)
                    {
                        send_hires_velocity = t2.arg(3).value().as_bool();
                    }
                }

                mapping_.alternate().settings_ = global_settings_t(minimum_decimation, send_notes, send_pitchbend, send_hires_velocity);
            }
            else
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
        piw::term_t t("mapping",mg.value().map_params_.size()+mg.value().map_midi_.size()+1);

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

        {
            piw::term_t t2("s",4);
            t2.set_arg(0,piw::makefloat(mg.value().settings_.minimum_decimation_));
            t2.set_arg(1,piw::makebool(mg.value().settings_.send_notes_));
            t2.set_arg(2,piw::makebool(mg.value().settings_.send_pitchbend_));
            t2.set_arg(3,piw::makebool(mg.value().settings_.send_hires_velocity_));
            t.set_arg(i,t2);
            ++i;
        }

        return t.render();
    }

    void controllers_mapping_t::map_param(unsigned iparam, mapping_info_t &info)
    {
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
        mapping_.alternate().settings_ = settings;
        mapping_.alternate().serial_++;
        mapping_.exchange();
        listener_.mapping_changed(get_mapping());
        listener_.settings_changed();
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
                    mapping_wrapper_t(ip.first->second.scale_, ip.first->second.lo_, ip.first->second.base_,
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
                mapping.midi_.insert(std::make_pair(ic.first->second.oparam_,
                    mapping_wrapper_t(ic.first->second.scale_, ic.first->second.lo_, ic.first->second.base_,
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
}
