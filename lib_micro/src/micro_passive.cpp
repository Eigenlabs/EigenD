
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

#include <lib_micro/micro_passive.h>
#include <lib_micro/micro_active.h>
#include <lib_micro/micro_usb.h>
#include <picross/pic_thread.h>

struct calibration_row_t
{
    unsigned key,sensor;
    unsigned short min,max;
    unsigned short points[BCTMICRO_CALTABLE_POINTS];
};

struct micro::passive_t::impl_t : micro::active_t::delegate_t
{
    impl_t(const char *name, unsigned decim);
    void kbd_raw(bool resync,const micro::active_t::rawkbd_t &);

    micro::active_t::rawkbd_t data_;
    active_t loop_;
    pic::poller_t poller_;
    pic::gate_t gate_;
    unsigned decim_;
    unsigned scancount_;
    bool insync_;
    calibration_row_t cal_row_;
};

void micro::passive_t::impl_t::kbd_raw(bool resync,const micro::active_t::rawkbd_t &keys)
{
    if(scancount_ == decim_)
    {
        if(gate_.isopen())
        {
            insync_ = false;
        }
        data_ = keys;
        gate_.open();
        scancount_ = 0;
    }

    scancount_++;
}

micro::passive_t::impl_t::impl_t(const char *name, unsigned decim): loop_(name,this), poller_(&loop_), decim_(decim), scancount_(0), insync_(true)
{
}

micro::passive_t::passive_t(const char *name, unsigned decim)
{
    impl_=new impl_t(name,decim);
}

micro::passive_t::~passive_t()
{
    delete impl_;
}

unsigned short micro::passive_t::get_rawkey(unsigned key, unsigned sensor)
{
    return impl_->data_.keys[key].c[sensor];
}

unsigned short micro::passive_t::get_breath()
{
    return impl_->data_.breath[0];
}

unsigned short micro::passive_t::get_strip()
{
    return impl_->data_.strip;
}

void micro::passive_t::set_ledcolour(unsigned key, unsigned colour)
{
    impl_->loop_.set_led(key,colour);
}

void micro::passive_t::set_ledrgb(unsigned key, unsigned r, unsigned g, unsigned b)
{
    set_ledcolour(key,((r&3)<<4)|((g&3)<<2)|((b&3)<<0));
}

bool micro::passive_t::wait()
{
    impl_->gate_.shut();
    impl_->gate_.untimedpass();
    return impl_->insync_;
}

void micro::passive_t::sync()
{
    impl_->insync_ = true;
}

std::string micro::passive_t::debug()
{
    return impl_->loop_.debug();
}

void micro::passive_t::start_calibration_row(unsigned key, unsigned corner)
{
    impl_->cal_row_.key=key;
    impl_->cal_row_.sensor=corner;
}

void micro::passive_t::set_calibration_range(unsigned short min, unsigned short max)
{
    impl_->cal_row_.min = min;
    impl_->cal_row_.max = max;
}

void micro::passive_t::set_calibration_point(unsigned point, unsigned short value)
{
    impl_->cal_row_.points[point] = value;
}

void micro::passive_t::write_calibration_row()
{
    impl_->loop_.set_calibration(impl_->cal_row_.key,impl_->cal_row_.sensor,impl_->cal_row_.min,impl_->cal_row_.max,impl_->cal_row_.points);
}

void micro::passive_t::commit_calibration()
{
    impl_->loop_.write_calibration();
}

void micro::passive_t::clear_calibration()
{
    impl_->loop_.clear_calibration();
}

void micro::passive_t::read_calibration_row()
{
    impl_->loop_.get_calibration(impl_->cal_row_.key,impl_->cal_row_.sensor,&impl_->cal_row_.min,&impl_->cal_row_.max,impl_->cal_row_.points);
}

unsigned short micro::passive_t::get_calibration_min()
{
    return impl_->cal_row_.min;
}

unsigned short micro::passive_t::get_calibration_max()
{
    return impl_->cal_row_.max;
}

unsigned short micro::passive_t::get_calibration_point(unsigned point)
{
    return impl_->cal_row_.points[point];
}

void micro::passive_t::start()
{
    impl_->loop_.start();
    impl_->poller_.start_polling();
}

void micro::passive_t::stop()
{
    impl_->poller_.stop_polling();
    impl_->loop_.stop();
}

unsigned micro::passive_t::get_temperature()
{
    return impl_->loop_.get_temperature();
}
