
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

#include <piw/piw_connector.h>
#include <memory>

piw::backend_delegate_t::backend_delegate_t(bool clocked): clocked_(clocked), latency_(0)
{
}

piw::backend_delegate_t::~backend_delegate_t()
{
    clear();
}

void piw::backend_delegate_t::clear()
{
    if(clock_source_.isvalid() && clocked_)
    {
        clock_source_->clear_downstream();
    }

    if(ctl_backend_.isvalid())
    {
        ctl_backend_->remove_latency();
    }

    if(data_backend_.isvalid())
    {
        data_backend_->remove_latency();
    }
}

piw::backend_t *piw::backend_delegate_t::get_data_backend()
{
    if(!data_backend_.isvalid())
    {
        data_backend_ = create_data_backend();

        if(data_backend_.isvalid())
        {
            data_backend_->set_latency(latency_);

            if(clock_source_.isvalid() && clocked_)
            {
                data_backend_->set_clock_source(clock_source_.ptr());
            }
        }
    }

    return data_backend_.ptr();
}

piw::backend_t *piw::backend_delegate_t::get_controller_backend()
{
    if(!ctl_backend_.isvalid())
    {
        ctl_backend_ = create_controller_backend();

        if(ctl_backend_.isvalid())
        {
            ctl_backend_->set_latency(latency_);

            //if(clock_source_.isvalid() && clocked_)
            //{
                //ctl_backend_->set_clock_source(clock_source_.ptr());
            //}
        }
    }

    return ctl_backend_.ptr();
}

void piw::backend_t::set_clock_source(piw::client_t *clock_source)
{
    if(correlator_.isvalid() && clock_source)
    {
        if(clock_source->set_downstream(correlator_->clocksink()))
        {
            correlator_->clock_plumbed(signal_,true);
        }
        else
        {
            pic::logmsg() << "bad clock for " << clock_source->id();
            correlator_->clock_plumbed(signal_,false);
        }
    }
}

void piw::backend_delegate_t::set_clock_source(piw::client_t *clock_source)
{
    if(clock_source_.isvalid() && clocked_)
    {
        clock_source_->clear_downstream();
    }

    clock_source_ = clock_source;

    if(clock_source_.isvalid() && clocked_)
    {
        //if(ctl_backend_.isvalid())
        //{
        //    ctl_backend_->set_clock_source(clock_source_.ptr());
        //}

        if(data_backend_.isvalid())
        {
            data_backend_->set_clock_source(clock_source_.ptr());
        }
    }
}

void piw::backend_delegate_t::set_clocked(bool clocked)
{
    if(clock_source_.isvalid() && clocked_)
    {
        clock_source_->clear_downstream();
    }

    clocked_ = clocked;

    if(clock_source_.isvalid() && clocked_)
    {
        //if(ctl_backend_.isvalid())
        //{
        //    ctl_backend_->set_clock_source(clock_source_.ptr());
        //}

        if(data_backend_.isvalid())
        {
            data_backend_->set_clock_source(clock_source_.ptr());
        }
    }
}

void piw::backend_delegate_t::set_latency(unsigned latency)
{
    latency_ = latency;

    if(ctl_backend_.isvalid()) ctl_backend_->set_latency(latency_);
    if(data_backend_.isvalid()) data_backend_->set_latency(latency_);
}

piw::backend_t::backend_t(correlator_t *correlator, unsigned iid, unsigned signal, int pri, unsigned type):
    correlator_(correlator), iid_(iid), signal_(signal), pri_(pri), type_(type)
{
}

piw::backend_t::~backend_t()
{
    tracked_invalidate();
}

void piw::backend_t::set_latency(unsigned l)
{
    if(correlator_.isvalid())
    {
        correlator_->set_latency(signal_,iid_,l);
    }
}

void piw::backend_t::remove_latency()
{
    if(correlator_.isvalid())
    {
        correlator_->remove_latency(signal_,iid_);
    }
}

struct piw::connector_t::impl_t: piw::client_t
{
    impl_t(bool ctl, piw::backend_delegate_t *backend, const piw::data_t &p, const piw::d2d_nb_t &filter, bool iso): ctl_(ctl), backend_(backend), xbackend_(0), path_(p), data2_(0), filter_(filter), iso_(iso)
    {
    }

    ~impl_t()
    {
        close_client();
    }

    void client_opened()
    {
        piw::client_t::client_opened();
        populate();

        if((get_host_flags()&PLG_SERVER_TRANSIENT)==0)
        {
            plumb();
        }
    }

    void close_client()
    {
        piw::client_t::close_client();
        unplumb();

        std::map<unsigned char, impl_t *>::iterator ci;

        while((ci = children_.begin()) != children_.end())
        {
            impl_t *p = ci->second;
            children_.erase(ci);
            delete p;
        }
    }

    void client_tree()
    {
        piw::client_t::client_tree();
        populate();
    }

    void plumb()
    {
        unplumb();

        if(ctl_ || (get_host_flags()&PLG_SERVER_CTL)!=0)
        {
            xbackend_ = backend_->get_controller_backend();
        }
        else
        {
            xbackend_ = backend_->get_data_backend();
        }

        if((get_host_flags()&PLG_SERVER_FAST)!=0)
        {
            data2_ = new piw::fastdata_t();
            piw::tsd_fastdata(data2_);
            set_sink(data2_);
        }
        else
        {
            unsigned long long t = piw::tsd_time();
            data2_ = new piw::fastdata_t(PLG_FASTDATA_SENDER);
            queue_ = piw::tsd_dataqueue(PIW_DATAQUEUE_SIZE_NORM);
            piw::tsd_fastdata(data2_);
            if(!get_data().is_null())
                queue_.write_slow(get_data());
            data2_->send_slow(piw::pathnull(t),queue_);
        }

        //pic::logmsg() << "plumbing to correlator for signal " << backend_->signal_ << " at path " << path_ << " correlator=" << backend_->correlator_;
        if(xbackend_.isvalid() && xbackend_->correlator_.isvalid())
        {
            converter_ = xbackend_->create_converter(iso_);
            xbackend_->correlator_->plumb_input(xbackend_->signal_,xbackend_->iid_,path_,xbackend_->pri_,xbackend_->type_,data2_,converter_,filter_);
        }
    }

    void client_data(const piw::data_t &d)
    {
        piw::tsd_fastcall(__data,this,(void *)d.give_copy());
    }

    static int __data(void *i_, void *d_)
    {
        impl_t *i = (impl_t *)i_;
        if(i->queue_.isvalid())
        {
            unsigned long long t = piw::tsd_time();
            piw::data_nb_t d = piw::data_nb_t::from_given((bct_data_t)d_);
            i->queue_.write_fast(d.restamp(t));
            //pic::logmsg() << "data " << d << " for signal" << i->signal_ << " at path " << i->path_ << " connector=" << (void *)i->connector_ << " correlator=" << i->correlator_;
        }
        return 0;
    }

    void unplumb()
    {
        if(data2_)
        {
            if(xbackend_.isvalid() && xbackend_->correlator_.isvalid())
            {
                xbackend_->correlator_->unplumb_input(xbackend_->signal_,xbackend_->iid_,path_,xbackend_->pri_);
            }

            converter_.clear();
            clear_sink();
            delete data2_;
            data2_ = 0;
            xbackend_ = 0;
        }

        queue_.clear();
    }

    void plumb_clock(piw::client_t *c)
    {
        backend_->set_clock_source(c);
    }

    void populate()
    {
        unsigned char i = enum_child(0);

        while(i)
        {
            if(!child_get(&i,1))
            {
                piw::data_t p2(piw::pathappend_channel(path_,i));
                std::auto_ptr<impl_t> p(new impl_t(ctl_,backend_.ptr(),p2,filter_,iso_));
                child_add(i, p.get());
                children_[i] = p.release();
            }

            i = enum_child(i);
        }

        std::map<unsigned char, impl_t *>::iterator ci, ce;

    prune:

        ci = children_.begin();
        ce = children_.end();

        for(; ci != ce; ++ci)
        {
            i = ci->first;

            if(!child_get(&i,1))
            {
                impl_t *p = ci->second;
                children_.erase(ci);
                delete p;
                goto prune;
            }
        }
    }

    bool ctl_;
    pic::weak_t<piw::backend_delegate_t> backend_;
    pic::weak_t<piw::backend_t> xbackend_;
    piw::data_t path_;
    std::map<unsigned char, impl_t *> children_;
    piw::fastdata_t *data2_;
    converter_ref_t converter_;
    piw::dataqueue_t queue_;
    piw::d2d_nb_t filter_;
    bool iso_;
};

piw::connector_t::connector_t(bool ctl, piw::backend_delegate_t *backend, const piw::d2d_nb_t &filter,bool iso): piw::client_t(PLG_CLIENT_CLOCK)
{
    impl_ = new impl_t(ctl,backend,piw::pathnull(0),d2d_nb_t::indirect(filter),iso);
}

piw::connector_t::~connector_t()
{
    close_client();
    delete impl_;
}

void piw::connector_t::client_clock()
{
    piw::client_t::client_clock();
    impl_->plumb_clock(this);
}

void piw::connector_t::client_opened()
{
    piw::client_t::client_opened();
    clone(impl_);
    impl_->plumb_clock(this);
}

void piw::connector_t::close_client()
{
    piw::client_t::close_client();
    impl_->close_client();
}

int piw::connector_t::gc_traverse(void *v, void *a) const
{
    return impl_->filter_.gc_traverse(v,a);
}

int piw::connector_t::gc_clear()
{
    pic::indirect_clear(impl_->filter_);
    return 0;
}
