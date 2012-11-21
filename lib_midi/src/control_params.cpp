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
#include <lib_midi/midi_gm.h>

namespace midi
{
    /**
     * param_wire_t
     */

    param_wire_t::param_wire_t(input_root_t *root, const piw::event_data_source_t &es): root_(root), path_(es.path()), id_(), ended_(false), processed_data_(false)
    {
        subscribe_and_ping(es);
    }

    param_wire_t::~param_wire_t()
    {
        invalidate();
    }

    void param_wire_t::wire_closed()
    {
        invalidate();
        delete this;
    }

    void param_wire_t::invalidate()
    {
        if(root_)
        {
            root_->wires_.alternate().erase(path_);
            root_->wires_.exchange();

            unsubscribe();
            wire_t::disconnect();
            piw::tsd_fastcall(__clear,this,0);
            root_ = 0;
        }
    }

    int param_wire_t::__clear(void *a, void *b)
    {
        param_wire_t *w = (param_wire_t *)a;
        w->iterator_.clear();
        w->root_->active_.remove(w);
        w->root_->rotating_active_.remove(w);
        return 0;
    }

    void param_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
    {
        ended_ = false;
        iterator_ = b.iterator();
        iterator_->reset_all(id.time());
        id_.set_nb(id);
        channel_ = root_->get_active_midi_channel(id);
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

    input_root_t::input_root_t(clocking_delegate_t *d): piw::root_t(0), clocking_delegate_(d), clock_(0)
    {
    }

    input_root_t::~input_root_t()
    {
        invalidate();
    }

    void input_root_t::invalidate()
    {
        piw::root_t::disconnect();

        param_wire_map_t::iterator wi;

        while((wi=wires_.alternate().begin()) != wires_.alternate().end())
        {
            delete wi->second;
        }

        if(clock_)
        {
            if(clocking_delegate_.isvalid())
            {
                clocking_delegate_->remove_upstream_clock(clock_);
            }
            clock_ = 0;
        }
    }

    void input_root_t::root_clock()
    {
        if(clock_)
            clocking_delegate_->remove_upstream_clock(clock_);

        clock_ = get_clock();

        if(clock_)
            clocking_delegate_->add_upstream_clock(clock_);
    }

    piw::wire_t *input_root_t::root_wire(const piw::event_data_source_t &es)
    {
        piw::data_t path = es.path();

        param_wire_map_t::iterator i = wires_.alternate().find(path);
        if(i != wires_.alternate().end())
        {
            delete i->second;
        }

        param_wire_t *w = new param_wire_t(this,es);
        wires_.alternate().insert(std::make_pair(path,w));
        wires_.exchange();
        return w;
    }


    /*
     * param_input_t
     */

    param_input_t::param_input_t(params_delegate_t *d, unsigned name): input_root_t(d), params_delegate_(d), control_mapping_(name), current_id_(piw::makenull(0)), current_data_(piw::makenull(0))
    {
    }

    param_input_t::~param_input_t()
    {
        invalidate();
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

    unsigned param_input_t::get_active_midi_channel(const piw::data_nb_t &id)
    {
        return params_delegate_->get_active_midi_channel(id);
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
        cleanup_wire(w);

        w->ended_ = true;
        w->processed_data_ = false;
    }

    void param_input_t::cleanup_wire(param_wire_t *w)
    {
        end_with_origins(w, (w==active_.head()));
        cleanup_params(w->get_id());
        cleanup_midi(w->get_id());
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
                if(PERNOTE_SCOPE==ip->second.scope_ && w->channel_ > 0)
                {
                    params.push_back(param_data_t(io->first+w->channel_-1, io->second, PERNOTE_SCOPE));
                }
                else
                {
                    params.push_back(param_data_t(io->first, io->second, ip->second.scope_));
                }
            }
        }

        if(!params.empty())
        {
            params_delegate_->set_parameters(params);
        }
    }

    bool param_input_t::wiredata_processed(param_wire_t *w, const piw::data_nb_t &d)
    {
        return true;
    }

    float param_input_t::calculate_param_value(const piw::data_nb_t &id, const piw::data_nb_t &d, const mapping_data_t &mapping, const float origin)
    {
        return origin + mapping.calculate(d.as_norm());
    }

    long param_input_t::calculate_midi_value(const piw::data_nb_t &id, const piw::data_nb_t &d, const mapping_data_t &mapping)
    {
        return mapping.calculate(d.as_norm()) * 16383.f;
    }

    void param_input_t::resend_current(const piw::data_nb_t &context)
    {
        piw::data_nb_t id = current_id_;
        piw::data_nb_t d = current_data_;
        unsigned channel = current_channel_;

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
                    process_midi(midi, channel, id, d, false, false, false);
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
        current_channel_ = w->channel_;

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

        process_params(params, current_channel_, current_id_, current_data_, first_wire, ending);
        process_midi(midi, current_channel_, current_id_, current_data_, continuous, true, ending);

        return continuous;
    }

    void param_input_t::process_params(pic::lckvector_t<param_data_t>::nbtype &params, unsigned channel, const piw::data_nb_t &id, const piw::data_nb_t &d, bool first_wire, bool ending)
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
                value = calculate_param_value(id, d, ip->second, origin);
            }

            if(value<0) value = 0;
            if(value>1) value = 1;

            unsigned long long current_time = piw::tsd_time();
            if(!ending && !ip->second.valid_for_processing(id, current_time))
            {
                continue;
            }

            if(PERNOTE_SCOPE==ip->second.scope_ && channel > 0)
            {
                params.push_back(param_data_t(ip->first+channel-1, value, PERNOTE_SCOPE));
            }
            else
            {
                params.push_back(param_data_t(ip->first, value, ip->second.scope_));
            }
        }
    }

    void param_input_t::process_midi(pic::lckvector_t<midi_data_t>::nbtype &midi, unsigned channel, const piw::data_nb_t &id, const piw::data_nb_t &d, bool continuous, bool accept_global_scope, bool ending)
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
                if(mid == MIDI_CC_MAX+midi::PITCH_WHEEL)
                {
                    value = 0x2000;
                }
                else
                {
                    value = 0;
                }
            }
            else
            {
                // if the data was generated because at event end there was nothing to process
                // don't send out the value if a return to origin wasn't configured
                if(ending && d.is_null())
                {
                    continue;
                }
                value = calculate_midi_value(id, d, ic->second);
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
            if(!ending && !ic->second.valid_for_processing(id, current_time))
            {
                continue;
            }

            // make sure that the value in the ending state is not send as continuous
            midi.push_back(midi_data_t(d.time(), mid, lid, value, ic->second.scope_, ic->second.channel_, channel, id, !ending && continuous));
        }
    }

    void param_input_t::cleanup_params(const piw::data_nb_t &id)
    {
        nb_param_map_t::iterator ip,bp,ep;
        bp = control_mapping_.params().begin();
        ep = control_mapping_.params().end();

        for(ip=bp; ip!=ep; ++ip)
        {
            ip->second.done_processing(id);
        }
    }

    void param_input_t::cleanup_midi(const piw::data_nb_t &id)
    {
        nb_midi_map_t::iterator ic,bc,ec;
        bc = control_mapping_.midi().begin();
        ec = control_mapping_.midi().end();

        for(ic=bc; ic!=ec; ++ic)
        {
            ic->second.done_processing(id);
        }
    }

} // namespace midi
