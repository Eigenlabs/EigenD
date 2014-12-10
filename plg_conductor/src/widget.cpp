
#include "widget.h"
#include <map>

namespace
{
    struct invocation_t: pic::atomic_counted_t
    {
        piw::change_t callback;
        bool completed;
        piw::data_t result;
    };
}

class cdtr::widget_t::impl_t: public pic::tracked_t
{
    public:
        impl_t(widget_t *host, const piw::change_t &receiver);
        ~impl_t();

        cdtr::widget_t *host_;
        piw::change_t receiver_;
        pic::weak_t<cdtr::widget_provider_t::impl_t> provider_;
};

class cdtr::widget_provider_t::impl_t: public pic::tracked_t
{
    public:
        impl_t(widget_provider_t *host);
        ~impl_t();

        cdtr::widget_provider_t *host_;
        pic::weak_t<cdtr::widget_t::impl_t> consumer_;

        std::map<void *, pic::ref_t<invocation_t> > invocations_;
};

cdtr::widget_t::widget_t(const piw::change_t &receiver): impl_(new impl_t(this,receiver))
{
}

cdtr::widget_t::~widget_t()
{
    delete impl_;
}

cdtr::widget_t::impl_t::impl_t(widget_t *host, const piw::change_t &receiver): host_(host), receiver_(receiver)
{
}

cdtr::widget_t::impl_t::~impl_t()
{
    tracked_invalidate();
    pic::weak_t<widget_provider_t::impl_t>::guard_t p(provider_);

    if(p.value())
    {
        std::map<void *,pic::ref_t<invocation_t> >::iterator i;

        for(i=p.value()->invocations_.begin(); i!=p.value()->invocations_.end(); i++)
        {
            i->second->callback = piw::change_t();
        }
    }
}

cdtr::widget_provider_t::impl_t::impl_t(widget_provider_t *host): host_(host)
{
}

cdtr::widget_provider_t::impl_t::~impl_t()
{
    tracked_invalidate();

    std::map<void *,pic::ref_t<invocation_t> >::iterator i;

    for(i=invocations_.begin(); i!= invocations_.end(); i++)
    {
        i->second->callback(piw::data_t());
    }

}

void cdtr::widget_t::change_value(const piw::data_t &value)
{
    pic::weak_t<widget_provider_t::impl_t>::guard_t p(impl_->provider_);

    if(p.value())
    {
        p.value()->host_->value_changed(value);
    }
}

bool cdtr::widget_t::invoke_rpc(int method, const piw::data_t &args, const piw::change_t &callback)
{
    pic::weak_t<widget_provider_t::impl_t>::guard_t p(impl_->provider_);

    if(!p.value())
    {
        return false;
    }


    pic::ref_t<invocation_t> invocation = pic::ref(new invocation_t);

    invocation->completed = false;
    invocation->callback = callback;

    p.value()->invocations_.insert(std::make_pair(invocation.ptr(),invocation));

    if(!p.value()->host_->rpc_invoked(invocation.ptr(),method,args))
    {
        p.value()->invocations_.erase(invocation.ptr());
        return false;
    }

    return true;
}

cdtr::widget_provider_t::widget_provider_t(): impl_(new impl_t(this))
{
}

cdtr::widget_provider_t::~widget_provider_t()
{
    delete impl_;
}

void cdtr::widget_provider_t::initialise_widget(widget_t *widget)
{
    impl_->consumer_ = widget->impl_;
    impl_->consumer_->provider_ = impl_;
}

void cdtr::widget_provider_t::change_value(const piw::data_t &value)
{
    pic::weak_t<widget_t::impl_t>::guard_t p(impl_->consumer_);

    if(p.value())
    {
        p.value()->receiver_(value);
    }
}

void cdtr::widget_provider_t::complete_rpc_string(void *context, const char *result)
{
    std::map<void *,pic::ref_t<invocation_t> >::iterator i;

    if((i=impl_->invocations_.find(context)) != impl_->invocations_.end())
    {
        i->second->completed = true;
        i->second->result = result?piw::makestring(result,0):piw::data_t();
        i->second->callback(i->second->result);
        impl_->invocations_.erase(i);
    }
}
