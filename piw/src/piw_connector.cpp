
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

struct piw::connector_t::impl_t: piw::client_t
{
    impl_t(connector_t *r,correlator_t *c, unsigned iid,unsigned s, int pri, unsigned type, const piw::data_t &p, const d2d_nb_t &f,bool clock): connector_(r), correlator_(c), iid_(iid), path_(p), signal_(s), filter_(f), pri_(pri), data2_(0), clock_(clock), type_(type)
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

        converter_=connector_->create_converter();

        if((get_host_flags()&PLG_SERVER_FAST)!=0)
        {
            data2_=new piw::fastdata_t();
            piw::tsd_fastdata(data2_);
            set_sink(data2_);
        }
        else
        {
            unsigned long long t = piw::tsd_time();
            data2_=new piw::fastdata_t(PLG_FASTDATA_SENDER);
            queue_=piw::tsd_dataqueue(PIW_DATAQUEUE_SIZE_NORM);
            piw::tsd_fastdata(data2_);
            if(!get_data().is_null())
                queue_.write_slow(get_data());
            data2_->send_slow(piw::pathnull(t),queue_);
        }

        //pic::logmsg() << "plumbing to correlator for signal " << signal_ << " at path " << path_ << " connector=" << (void *)connector_ << " correlator=" << correlator_;
        correlator_->plumb_input(signal_,iid_,path_,pri_,type_,data2_,converter_,filter_);
    }

    void client_data(const piw::data_nb_t &d)
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
            correlator_->unplumb_input(signal_,iid_,path_,pri_);
            converter_.clear();
            clear_sink();
            delete data2_;
            data2_=0;
        }

        queue_.clear();
    }

    void plumb_clock(piw::client_t *c)
    {
        if(clock_)
        {
            if(c->set_downstream(correlator_->clocksink()))
            {
                //pic::logmsg() << "good clock for " << id();
                correlator_->clock_plumbed(signal_,true);
            }
            else
            {
                //pic::logmsg() << "bad clock for " << id();
                correlator_->clock_plumbed(signal_,false);
            }
        }
    }

    void populate()
    {
        unsigned char i = enum_child(0);

        while(i)
        {
            if(!child_get(&i,1))
            {
                piw::data_t p2(piw::pathappend_chaff(path_,i));
                std::auto_ptr<impl_t> p(new impl_t(connector_,correlator_,iid_,signal_,pri_,type_,p2,filter_,clock_));
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

    piw::connector_t *connector_;
    piw::correlator_t *correlator_;
    unsigned iid_;
    piw::data_t path_;
    unsigned signal_;
    piw::d2d_nb_t filter_;
    int pri_;
    std::map<unsigned char, impl_t *> children_;
    piw::fastdata_t *data2_;
    converter_ref_t converter_;
    bool clock_;
    unsigned type_;
    piw::dataqueue_t queue_;
};

piw::connector_t::connector_t(correlator_t *correlator, unsigned iid, unsigned signal, int pri, unsigned type, const d2d_nb_t &filter,bool clock): piw::client_t(PLG_CLIENT_CLOCK)
{
    d2d_nb_t tmp = d2d_nb_t::indirect(filter);
    impl_ = new impl_t(this,correlator,iid,signal,pri,type,piw::pathnull(0),tmp,clock);
}

piw::connector_t::~connector_t()
{
    close_client();
    delete impl_;
}

void piw::connector_t::set_clocked(bool c)
{
    impl_->clock_ = c;
    impl_->plumb_clock(this);
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
