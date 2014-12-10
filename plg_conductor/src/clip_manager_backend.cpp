
#include "clip_manager_backend.h"
#include <piw/piw_tsd.h>

namespace
{
    struct XmlElementHolder
    {
        XmlElementHolder(String text): element(XmlDocument::parse(text))
        {
        }

        ~XmlElementHolder()
        {
            delete element;
        }

        operator XmlElement *()
        {
            return element;
        }

        XmlElement *operator->()
        {
            return element;
        }

        XmlElement *element;
    };

    struct rpc_dispatch_t: public pic::nocopy_t
    {
        rpc_dispatch_t(cdtr::clip_manager_backend_t *cl,void *c, int m, const piw::data_t &a): clip(cl), context(c), method(m), arguments(a)
        {
        }

        cdtr::clip_manager_backend_t *clip;
        void *context;
        int method;
        piw::data_t arguments;
    };
};

cdtr::clip_manager_backend_t::~clip_manager_backend_t()
{
    quit();
    close_thing();
}

cdtr::clip_manager_backend_t::clip_manager_backend_t(): pic::safe_worker_t(1000,PIC_THREAD_PRIORITY_NORMAL), sequence_(0)
{
    piw::tsd_thing(this);
    run();
}

void cdtr::clip_manager_backend_t::bump_sequence()
{
    std::cout<<"bump_sequence"<<std::endl;
    trigger_slow();
}

void cdtr::clip_manager_backend_t::value_changed(const piw::data_t &value)
{
    std::cout<<"clip_manager_backend_t: value_changed"<<std::endl;
}

void cdtr::clip_manager_backend_t::thing_trigger_slow()
{
    std::cout<<"sequence incremented - change value called"<<std::endl;
    sequence_ ++;
    char buffer[16];
    sprintf(buffer,"%u",sequence_);
    change_value(piw::makestring(buffer,0));
}

void cdtr::clip_manager_backend_t::complete_rpc(void *context, const XmlElement *value)
{
    String marshalled = "";

    marshalled << (int64)context;
    marshalled << ":";

    if(value)
    {
        marshalled << value->createDocument("");
    }

    std::cout << "queueing complete " << marshalled << std::endl;

    enqueue_slow(piw::makestring(marshalled.toUTF8(),0));
}

void cdtr::clip_manager_backend_t::thing_dequeue_slow(const piw::data_t &d)
{
    String marshalled = String::fromUTF8(d.as_string());
    int i = marshalled.indexOfChar(':');

    String cstring = marshalled.substring(0,i);
    String xstring = marshalled.substring(i+1);

    void *context = (void *)cstring.getLargeIntValue();

    pic::logmsg() << "running complete " << (void *)context << " " << xstring;

    complete_rpc_string(context,xstring.toUTF8());
}

bool cdtr::clip_manager_backend_t::rpc_invoked(void *context, int method, const piw::data_t &arguments)
{
    rpc_dispatch_t *d = new rpc_dispatch_t(this,context,method,arguments);
    add(dispatch__,d,0,0,0);
    return true;
}

void cdtr::clip_manager_backend_t::dispatch__(void *d_,void *,void *,void *)
{
    rpc_dispatch_t *d = (rpc_dispatch_t *)d_;

    int method = d->method;
    piw::data_t arguments = d->arguments;
    void *context = d->context;
    clip_manager_backend_t *self = d->clip;

    delete d;

    try
    {
        XmlElementHolder e(arguments.as_string());

        switch(method)
        {
            //case 1: if(self->rpc_set_selected_tags(context,e)) return;
            case 2: if(self->rpc_change_tags_for_clip(context,e)) return;
            case 3: if(self->rpc_get_column_categories(context,e)) return;
            case 4: if(self->rpc_get_all_tags(context,e)) return;
            case 5: if(self->rpc_get_selected_clips(context,e)) return;
            case 6: if(self->rpc_add_to_clip_pool(context,e,e->getIntAttribute("param_index"))) return;
            case 7: if(self->rpc_add_tag(context,e)) return;
            case 8: if(self->rpc_change_tag(context,e)) return;
            case 9: if(self->rpc_remove_tag(context,e)) return;
            case 10: if(self->rpc_add_category(context,e)) return;
        }
    }
    catch(...)
    {
    }

    self->complete_rpc(context,0);
}
