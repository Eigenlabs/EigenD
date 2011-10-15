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

#include <lib_midi/control_params.h>

namespace midi
{
    /**
     * param_wire_t
     */

    param_wire_t::param_wire_t(input_root_t *root, const piw::event_data_source_t &es): root_(root), path_(es.path()), id_(), ended_(false), processed_data_(false)
    {
        root_->wires_.alternate().insert(std::make_pair(path_,this));
        root_->wires_.exchange();
        subscribe_and_ping(es);
    }

    param_wire_t::~param_wire_t()
    {
        invalidate();
    }

    void param_wire_t::wire_closed()
    {
        delete this;
    }

    void param_wire_t::invalidate()
    {
        if(root_)
        {
            piw::tsd_fastcall(__clear,this,0);
            unsubscribe();
            root_->wires_.alternate().erase(path_);
            root_->wires_.exchange();
            root_ = 0;
        }
    }

    int param_wire_t::__clear(void *a, void *b)
    {
        param_wire_t *w = (param_wire_t *)a;
        w->iterator_.clear();
        return 0;
    }

    void param_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
    {
        ended_ = false;
        iterator_ = b.iterator();
        iterator_->reset_all(id.time());
        id_.set_nb(id);
        root_->started(this);
        root_->active_.append(this);
        root_->rotating_active_.append(this);
    }

    bool param_wire_t::event_end(unsigned long long t)
    {
        root_->ending(this, t);

        iterator_.clear();
        pic::element_t<0>::remove();
        pic::element_t<1>::remove();
        if(root_->active_.head() && root_->active_.head()->iterator_.isvalid())
        {
            root_->active_.head()->iterator_->reset_all(t); // catch up the next one
        }
        root_->ended(this);
        
        return true;
    }

    void param_wire_t::event_buffer_reset(unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
    {
        iterator_->set_signal(sig,n);
        iterator_->reset(sig,t);
    }


    /*
     * input_root_t
     */

    input_root_t::input_root_t(clocking_delegate_t *d): piw::root_t(0), clocking_delegate_(d), clk_(0)
    {
    }

    input_root_t::~input_root_t()
    {
        while(!wires_.alternate().empty())
        {
            delete wires_.alternate().begin()->second;
        }
        wires_.exchange();
    }

    void input_root_t::root_clock()
    {
        if(clk_)
            clocking_delegate_->remove_upstream_clock(clk_);

        clk_ = get_clock();

        if(clk_)
            clocking_delegate_->add_upstream_clock(clk_);
    }

    piw::wire_t *input_root_t::root_wire(const piw::event_data_source_t &es)
    {
        piw::data_t path = es.path();
        param_wire_map_t::const_iterator i = wires_.alternate().find(path);

        if(i != wires_.alternate().end())
            delete i->second;

        return new param_wire_t(this,es);
    }


    /*
     * param_input_t
     */

    param_input_t::param_input_t(params_delegate_t *d, unsigned name): input_root_t(d), params_delegate_(d), control_mapping_(name), current_id_(piw::makenull(0)), current_data_(piw::makenull(0))
    {
    }

    void param_input_t::update_origins()
    {
        if(active_.head()) return;

        params_delegate_->update_origins(control_mapping_);
    }

    void param_input_t::update_mapping()
    {
        params_delegate_->update_mapping(control_mapping_);
    }

    void param_input_t::started(param_wire_t *w)
    {
        if(active_.head()) return;

        update_origins();
        update_mapping();
    }

    void param_input_t::ending(param_wire_t *w, unsigned long long to)
    {
        if(!w->processed_data_ || !active_.head() || w->ended_) return;

        // send last data on this wire before it ends
        pic::lckvector_t<param_data_t>::nbtype params;
        pic::lckvector_t<midi_data_t>::nbtype midi;

        process_wire(w, params, midi, to, (w==active_.head()), true);

        if(!params.empty()) params_delegate_->set_parameters(params);
        if(!midi.empty()) params_delegate_->set_midi(midi);
    }

    void param_input_t::ended(param_wire_t *w)
    {
        end_with_origins(w, (w==active_.head()));

        w->ended_ = true;
        w->processed_data_ = false;
    }

    void param_input_t::end_with_origins(param_wire_t *w, bool first_wire)
    {
        pic::lckvector_t<param_data_t>::nbtype params;

        control_mapping_.touch_origins_update();

        nb_param_map_t::iterator ip,bp,ep;
        bp = control_mapping_.params().begin();
        ep = control_mapping_.params().end();

        nb_origin_map_t::iterator io,bo,eo;
        bo = control_mapping_.origins().begin();
        eo = control_mapping_.origins().end();

        io=bo;

        for(ip=bp; ip!=ep; ++ip)
        {
            while(io->first < ip->first && io!=eo)
            {
                io++;
            }

            if(GLOBAL_SCOPE==ip->second.scope_ && !first_wire)
            {
                continue;
            }

            if(io!=eo && io->first==ip->first &&
               ip->second.origin_return_)
            {
                params.push_back(param_data_t(io->first, io->second, ip->second.scope_, w->id_.get(), extract_keynum(w->id_.get())));
            }
        }

        if(!params.empty())
        {
            params_delegate_->set_parameters(params);
        }
    }

    float param_input_t::calculate_param_value(const piw::data_nb_t &id, const float d, const mapping_data_t &mapping, const float origin)
    {
        return origin + mapping.calculate(d);
    }

    long param_input_t::calculate_midi_value(const piw::data_nb_t &id, const float d, const mapping_data_t &mapping)
    {
        return mapping.calculate(d) * 16383.f;
    }

    unsigned char param_input_t::extract_keynum(const piw::data_nb_t &id)
    {
        unsigned char keynum = 0;
        unsigned lp = id.as_pathgristlen();
        if(lp>0)
        {
            const unsigned char *p = id.as_pathgrist();
            keynum = p[lp-1];
        }
        return keynum;
    }

    void param_input_t::resend_current(const piw::data_nb_t &context)
    {
        piw::data_nb_t id = current_id_;
        piw::data_nb_t d = current_data_;

        if(!id.is_null() && !d.is_null())
        {
            const unsigned char *p = context.as_path();
            int l = context.as_pathlen();

            for(;l>=0;--l)
            {
                piw::data_nb_t d = piw::makepath_nb(p,l);

                if(0==id.compare_path(d))
                {
                    pic::lckvector_t<midi_data_t>::nbtype midi;
                    process_midi(midi, id, d, false, false, false);
                    if(!midi.empty())
                    {
                        params_delegate_->set_midi(midi);
                    }
                    return;
                }
            }
        }
    }

    void param_input_t::schedule(unsigned long long from, unsigned long long to)
    {
        param_wire_t *wh = rotating_active_.head();

        if(!wh) return;

        update_mapping();

        pic::lckvector_t<param_data_t>::nbtype params;
        pic::lckvector_t<midi_data_t>::nbtype midi;

        param_wire_t *w = wh;
        while(w)
        {
            w->processed_data_ = true;
            process_wire(w, params, midi, to, (w==active_.head()), false);
            w = active_.next(w);
        }
        rotating_active_.append(wh);

        if(!params.empty())
        {
            params_delegate_->set_parameters(params);
        }

        if(!midi.empty())
        {
            params_delegate_->set_midi(midi);
        }
    }

    void param_input_t::process_wire(param_wire_t *w, pic::lckvector_t<param_data_t>::nbtype &params, pic::lckvector_t<midi_data_t>::nbtype &midi, unsigned long long to, bool first_wire, bool ending)
    {
        if(w->ended_) return;

        current_id_.set_nb(w->get_id());

        piw::data_nb_t d;
        bool more_data = w->iterator_->nextsig(1,d,to);
        // handle the case where the input is ending, but there's no more data
        // this allows return to original configurations to be processed
        if(!more_data && ending)
        {
            current_data_.set_nb(piw::makenull_nb(piw::tsd_time()));
            process_wire_data(w, params, midi, first_wire, true);
        }

        // process outstanding data
        while(more_data)
        {
            current_data_.set_nb(d);

            more_data = w->iterator_->nextsig(1,d,to);

            bool continuous = process_wire_data(w, params, midi, first_wire, ending && !more_data);
            if(!continuous)
            {
                break;
            }
        }
    }

    bool param_input_t::process_wire_data(param_wire_t *w, pic::lckvector_t<param_data_t>::nbtype &params, pic::lckvector_t<midi_data_t>::nbtype &midi, bool first_wire, bool ending)
    {
        bool continuous = wiredata_processed(w, current_data_);

        process_params(params, current_id_, current_data_, first_wire, ending);
        process_midi(midi, current_id_, current_data_, continuous, true, ending);

        return continuous;
    }

    void param_input_t::process_params(pic::lckvector_t<param_data_t>::nbtype &params, const piw::data_nb_t &id, const piw::data_nb_t &d, bool first_wire, bool ending)
    {
        nb_param_map_t::iterator ip,bp,ep;
        bp = control_mapping_.params().begin();
        ep = control_mapping_.params().end();

        nb_origin_map_t::iterator io,bo,eo;
        bo = control_mapping_.origins().begin();
        eo = control_mapping_.origins().end();

        io=bo;

        for(ip=bp; ip!=ep; ++ip)
        {
            if(GLOBAL_SCOPE==ip->second.scope_ && !first_wire)
            {
                continue;
            }

            float origin = 0;
            while(io->first < ip->first && io!=eo)
            {
                io++;
            }

            if(io!=eo && io->first==ip->first)
            {
                origin=io->second;
            }

            float value;
            if(ending && ip->second.origin_return_)
            {
                value = origin; 
            }
            else
            {
                // if the data was generated because at event end there was nothing to process
                // don't send out the value if a return to origin wasn't configured
                if(ending && d.is_null())
                {
                    continue;
                }
                value = calculate_param_value(id, d.as_norm(), ip->second, origin);
            }

            if(value<0) value = 0;
            if(value>1) value = 1;

            unsigned long long current_time = piw::tsd_time();
            if(!ending &&
               ip->second.decimation_ &&
               ip->second.last_processed_ + (ip->second.decimation_*1000 ) > current_time)
            {
                continue;
            }
            ip->second.last_processed_ = current_time;

            params.push_back(param_data_t(ip->first, value, ip->second.scope_, id, extract_keynum(id)));
        }
    }

    void param_input_t::process_midi(pic::lckvector_t<midi_data_t>::nbtype &midi, const piw::data_nb_t &id, const piw::data_nb_t &d, bool continuous, bool accept_global_scope, bool ending)
    {
        nb_midi_map_t::iterator ic,bc,ec;
        bc = control_mapping_.midi().begin();
        ec = control_mapping_.midi().end();

        for(ic=bc; ic!=ec; ++ic)
        {
            if(!accept_global_scope && GLOBAL_SCOPE==ic->second.scope_)
            {
                continue;
            }

            unsigned char mid = ic->first;
            long value;
            if(ending && ic->second.origin_return_)
            {
                value = 0;
            }
            else
            {
                // if the data was generated because at event end there was nothing to process
                // don't send out the value if a return to origin wasn't configured
                if(ending && d.is_null())
                {
                    continue;
                }
                value = calculate_midi_value(id, d.as_norm(), ic->second);
            }

            if(value<0)
            {
                value = 0;
            }
            if(value>0x3fff)
            {
                value = 0x3fff;
            }

            unsigned char lid = 0;
            if(BITS_14 == ic->second.resolution_)
            {
                if(ic->second.resolution_>0 && ic->second.resolution_<128)
                {
                    lid = ic->second.secondary_cc_;
                }
            }

            unsigned long long current_time = piw::tsd_time();
            if(!ending &&
               ic->second.decimation_ &&
               ic->second.last_processed_ + (ic->second.decimation_*1000 ) > current_time)
            {
                continue;
            }
            ic->second.last_processed_ = current_time;

            // make sure that the value in the ending state is not send as continuous
            midi.push_back(midi_data_t(d.time(), mid, lid, value, ic->second.scope_, ic->second.channel_, id, extract_keynum(id), !ending && continuous));
        }
    }



    /*
     * keynum_input_t
     */

    keynum_input_t::keynum_input_t(params_delegate_t *d, unsigned name): param_input_t(d, name)
    {
    }

    float keynum_input_t::calculate_param_value(const piw::data_nb_t &id, const float d, const mapping_data_t &mapping, const float origin)
    {
        return origin + mapping.calculate(extract_keynum(id));
    }

    long keynum_input_t::calculate_midi_value(const piw::data_nb_t &id, const float d, const mapping_data_t &mapping)
    {
        long value = extract_keynum(id) - 1;
        if(BITS_7 == mapping.resolution_)
        {
            value <<= 7;
        }
        return mapping.calculate(value);
    }

    bool keynum_input_t::wiredata_processed(param_wire_t *w, const piw::data_nb_t &d)
    {
        w->ended_ = true;
        return false;
    }
    
} // namespace midi
