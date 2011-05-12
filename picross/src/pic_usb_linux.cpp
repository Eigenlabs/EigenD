

#include <picross/pic_usb.h>

bool pic::usbdevice_t::add_bulk_out(bulk_out_pipe_t *p)
{
    return true;
}

bool pic::usbdevice_t::add_iso_in(iso_in_pipe_t *p)
{
	return true;
}

void pic::usbdevice_t::set_iso_out(iso_out_pipe_t *p)
{
}

pic::usbdevice_t::usbdevice_t(const char *name, unsigned iface)
{
}

void pic::usbdevice_t::start_pipes()
{
}

void pic::usbdevice_t::stop_pipes()
{
}

pic::usbdevice_t::~usbdevice_t()
{
}

bool pic::usbdevice_t::poll_pipe(unsigned long long t)
{
    return false;
}

void pic::usbdevice_t::control_in(unsigned char type, unsigned char req, unsigned short val, unsigned short ind, void *buffer, unsigned len, unsigned timeout)
{
}

void pic::usbdevice_t::control_out(unsigned char type, unsigned char req, unsigned short val, unsigned short ind, const void *buffer, unsigned len, unsigned timeout)
{
}

void pic::usbdevice_t::control(unsigned char type, unsigned char req, unsigned short val, unsigned short ind, unsigned timeout)
{
}

void pic::usbdevice_t::bulk_out_pipe_t::bulk_write(const void *data, unsigned len, unsigned timeout)
{
}

void pic::usbdevice_t::set_power_delegate(power_t *p)
{
}

void pic::usbdevice_t::detach()
{
}

const char *pic::usbdevice_t::name()
{
    return "";
}

pic::usbdevice_t::iso_out_guard_t::iso_out_guard_t(usbdevice_t *d): impl_(d->impl()), current_(0), guard_(0), dirty_(false)
{
}

pic::usbdevice_t::iso_out_guard_t::~iso_out_guard_t()
{
}

unsigned char *pic::usbdevice_t::iso_out_guard_t::advance()
{
    return 0;
}

pic::usbenumerator_t::usbenumerator_t(unsigned short v, unsigned short p, const f_string_t &a)
{
}

pic::usbenumerator_t::usbenumerator_t(unsigned short v, unsigned short p, const f_string_t &a, const f_string_t &r)
{
}

pic::usbenumerator_t::~usbenumerator_t()
{
}

void pic::usbenumerator_t::start()
{
}

void pic::usbenumerator_t::stop()
{
}

unsigned pic::usbenumerator_t::enumerate(unsigned short vendor, unsigned short product, const f_string_t &callback)
{
    return 0;
}

int pic::usbenumerator_t::gc_traverse(void *v,void *a) const
{
    return 0;
}

int pic::usbenumerator_t::gc_clear()
{
    return 0;
}
