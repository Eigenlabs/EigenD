/*
 * live_osc_handler.cpp
 *
 *  Created on: Jun 29, 2013
 *      Author: kodiak
 */

#include "live_osc.h"

#include <lib_lo/lo/lo.h>

namespace oscpad_plg {


const char *live_beat_handler_t::topic() { return "/live/beat";}
void live_beat_handler_t::messageHandler(const char *path, lo_message& msg)
{
//	pic::logmsg() << "live_osc_beat_msg::messageHandler";
	if(lo_message_get_argc(msg) > 0  && lo_message_get_types(msg)[0]==LO_INT32)
	{
		lo_arg** args= lo_message_get_argv(msg);
		int beat = args[0]->i;
//		pic::logmsg() << "live_osc_beat_msg::messageHandler:" << beat;
		processBeat(beat);
	}
}

const char *live_clip_info_handler_t::topic() { return "/live/clip/info";}
void live_clip_info_handler_t::messageHandler(const char *path, lo_message& msg)
{
//	pic::logmsg() << "live_osc_clip_info_msg::messageHandler";
	if(lo_message_get_argc(msg) >= 3
			&& lo_message_get_types(msg)[0]==LO_INT32
			&& lo_message_get_types(msg)[1]==LO_INT32
			&& lo_message_get_types(msg)[2]==LO_INT32)
	{
		lo_arg** args= lo_message_get_argv(msg);
		int track = args[0]->i;
		int clip = args[1]->i;
		int state = args[2]->i;
//		pic::logmsg() << "live_osc_clip_info_msg::messageHandler track :" << track << "clip" << clip << "state" << state;
		processClip(track,clip,(state_t) state);
	}
}

// /live/track/info iiiif 0 1 0 2 63072000.000000, track, armed, clip, playing, length
const char *live_track_info_handler_t::topic() { return "/live/track/info";}
void live_track_info_handler_t::messageHandler(const char *path, lo_message& msg)
{
//	pic::logmsg() << "live_track_info_handler_t::messageHandler";
	if(lo_message_get_argc(msg) >= 5
			&& lo_message_get_types(msg)[0]==LO_INT32
			&& lo_message_get_types(msg)[1]==LO_INT32
			&& lo_message_get_types(msg)[2]==LO_INT32
			&& lo_message_get_types(msg)[3]==LO_INT32
			&& lo_message_get_types(msg)[4]==LO_FLOAT
			)
	{
		lo_arg** args= lo_message_get_argv(msg);
		int track = args[0]->i;
		int armed = args[1]->i;
		int clip = args[2]->i;
		int state = args[3]->i;
		float length = args[4]->i;
		processClip(track,(bool) armed,clip, (state_t) state,length);
	}
}


live_arm_message_t::live_arm_message_t(int track, bool armed) : track_(track), armed_(armed)
{
}

const char *live_arm_message_t::topic()
{
	return "/live/arm";
}

lo_message live_arm_message_t::createMessage()
{
	//MSH think about ref count
    pic::logmsg() << "live_arm_message_t::createMessage() " << topic() << " - " << track_;
	lo_message m=lo_message_new();
	lo_message_add(m,"i",track_);
	lo_message_add(m,"i",(int) armed_); // enable/arm track
	return m;
}

live_clip_slot_message_t::live_clip_slot_message_t(int track, int clip) : track_(track), clip_(clip)
{

}
const char *live_clip_slot_message_t::topic() { return "/live/play/clipslot";}
lo_message live_clip_slot_message_t::createMessage()
{
    pic::logmsg() << "live_clip_slot_message_t::createMessage() " << topic() << " - " << track_ << "," << clip_;
	//MSH think about ref count
	lo_message m=lo_message_new();
	lo_message_add(m,"i",track_);
	lo_message_add(m,"i",clip_); // enable/arm track
	return m;
}

// /live/clip/info (track,clip)
live_clip_info_message_t::live_clip_info_message_t(int track, int clip) : track_(track), clip_(clip)
{

}
const char *live_clip_info_message_t::topic() { return "/live/clip/info";}
lo_message live_clip_info_message_t::createMessage()
{
    pic::logmsg() << "live_clip_info_message_t::createMessage() " << topic() << " - " << track_ << "," << clip_;
	//MSH think about ref count
	lo_message m=lo_message_new();
	lo_message_add(m,"i",track_);
	lo_message_add(m,"i",clip_); // enable/arm track
	return m;
}




} /* namespace oscpad_plg */
