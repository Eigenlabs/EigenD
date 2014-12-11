/*
 * live_osc_handler.cpp
 *
 *  Created on: Jun 29, 2013
 *      Author: kodiak
 */

#include "live_osc.h"

#include <lib_lo/lo/lo.h>

namespace livepad_plg {


const char *live_beat_handler_t::topic() { return "/live/beat";}
void live_beat_handler_t::messageHandler(const char *path, lo_message& msg)
{
//	pic::logmsg() << "live_osc_beat_msg::messageHandler";
	if(lo_message_get_argc(msg) >= 1  && lo_message_get_types(msg)[0]==LO_INT32)
	{
		lo_arg** args= lo_message_get_argv(msg);
		int beat = args[0]->i;
//		pic::logmsg() << "live_osc_beat_msg::messageHandler:" << beat;
		processBeat(beat);
	}
}

// /live/arm ii 1 0 (track, armed status)
const char *live_arm_handler_t::topic() { return "/live/arm";}
void live_arm_handler_t::messageHandler(const char *path, lo_message& msg)
{
//	pic::logmsg() << "live_arm_handler_t::messageHandler";
	if(lo_message_get_argc(msg) >=2
			&& lo_message_get_types(msg)[0]==LO_INT32
			&& lo_message_get_types(msg)[1]==LO_INT32
			)
	{
		lo_arg** args= lo_message_get_argv(msg);
		int track = args[0]->i;
		int armed = args[1]->i;
//		pic::logmsg() << "live_arm_handler_t::messageHandler:" << track << "," << armed;
		processArm(track,(bool) armed);
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

const char *live_name_clip_handler_t::topic() { return "/live/name/clip";}
void live_name_clip_handler_t::messageHandler(const char *path, lo_message& msg)
{
//	pic::logmsg() << "live_clip_name_handler_t::messageHandler";
	if(lo_message_get_argc(msg) >= 4
			&& lo_message_get_types(msg)[0]==LO_INT32
			&& lo_message_get_types(msg)[1]==LO_INT32
			&& lo_message_get_types(msg)[2]==LO_STRING
			&& lo_message_get_types(msg)[3]==LO_INT32)
	{
		lo_arg** args= lo_message_get_argv(msg);
		int track = args[0]->i;
		int clip = args[1]->i;
		std::string name=&args[2]->s;
		int  colour= args[3]->i;
//		pic::logmsg() << "live_name_clip_handler_t::messageHandler track :" << track << "clip" << clip << "name" << name;
		processClip(track,clip,name,colour);
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

// /remix/oscserver/startup i 1
const char *live_startup_handler_t::topic() { return "/remix/oscserver/startup";}
void live_startup_handler_t::messageHandler(const char *path, lo_message& msg)
{
//	pic::logmsg() << "live_osc_beat_msg::messageHandler";
	if(lo_message_get_argc(msg) >= 1  && lo_message_get_types(msg)[0]==LO_INT32)
	{
//		lo_arg** args= lo_message_get_argv(msg);
//		int arg = args[0]->i;
//		pic::logmsg() << "live_startup_handler_t::messageHandler:"'
		process();
	}
}


// /remix/oscserver/shutdown i 1
const char *live_shutdown_handler_t::topic() { return "/remix/oscserver/shutdown";}
void live_shutdown_handler_t::messageHandler(const char *path, lo_message& msg)
{
//	pic::logmsg() << "live_osc_beat_msg::messageHandler";
	if(lo_message_get_argc(msg) >= 1  && lo_message_get_types(msg)[0]==LO_INT32)
	{
//		lo_arg** args= lo_message_get_argv(msg);
//		int arg = args[0]->i;
//		pic::logmsg() << "live_shutdown_handler_t::messageHandler:";
		process();
	}
}




//////////////////////// MESSAGES /////////

live_arm_message_t::live_arm_message_t(int track, bool armed) : track_(track), armed_(armed)
{
}

const char *live_arm_message_t::topic()
{
	return "/live/arm";
}

lo_message live_arm_message_t::createMessage()
{
    //pic::logmsg() << "live_arm_message_t::createMessage() " << topic() << " - " << track_;
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
    // pic::logmsg() << "live_clip_slot_message_t::createMessage() " << topic() << " - " << track_ << "," << clip_;
	lo_message m=lo_message_new();
	lo_message_add(m,"i",track_);
	lo_message_add(m,"i",clip_);
	return m;
}

// /live/clip/info (track,clip)
live_clip_info_message_t::live_clip_info_message_t(int track, int clip) : track_(track), clip_(clip)
{

}
const char *live_clip_info_message_t::topic() { return "/live/clip/info";}
lo_message live_clip_info_message_t::createMessage()
{
    // pic::logmsg() << "live_clip_info_message_t::createMessage() " << topic() << " - " << track_ << "," << clip_;
	lo_message m=lo_message_new();
	lo_message_add(m,"i",track_);
	lo_message_add(m,"i",clip_);
	return m;
}



// /live/clip/name
live_name_clip_message_t::live_name_clip_message_t()
{
}
const char *live_name_clip_message_t::topic() { return "/live/name/clip";}
lo_message live_name_clip_message_t::createMessage()
{
    // pic::logmsg() << "live_name_clip_message_t::createMessage() " << topic() ;
	lo_message m=lo_message_new();
	return m;
}

// /live/selection (track, clip, width, height)
live_selection_message_t::live_selection_message_t(unsigned track, unsigned clip, unsigned width,unsigned height)
	: track_(track), clip_(clip), width_(width), height_(height)
{
}
const char *live_selection_message_t::topic() { return "/live/selection";}
lo_message live_selection_message_t::createMessage()
{
    // pic::logmsg() << "live_selection_message_t::createMessage() " << topic() << " - " << track_ << "," << clip_ << "," << width_ << "," << height_;
	lo_message m=lo_message_new();
	lo_message_add(m,"i",(int) track_);
	lo_message_add(m,"i",(int) clip_);
	lo_message_add(m,"i",(int) width_);
	lo_message_add(m,"i",(int) height_);
	return m;
}

// /live/play
live_play_message_t::live_play_message_t()
{
}
const char *live_play_message_t::topic() { return "/live/play";}
lo_message live_play_message_t::createMessage()
{
    // pic::logmsg() << "live_play_message_t::createMessage() " << topic() ;
	lo_message m=lo_message_new();
	return m;
}

// /live/stop
live_stop_message_t::live_stop_message_t()
{
}
const char *live_stop_message_t::topic() { return "/live/stop";}
lo_message live_stop_message_t::createMessage()
{
    // pic::logmsg() << "live_stop_message_t::createMessage() " << topic() ;
	lo_message m=lo_message_new();
	return m;
}

// /live/play/scene
live_play_scene_message_t::live_play_scene_message_t(int scene) : scene_(scene)
{
}
const char *live_play_scene_message_t::topic() { return "/live/play/scene";}
lo_message live_play_scene_message_t::createMessage()
{
    // pic::logmsg() << "live_play_scene_message_t::createMessage() " << topic() <<"-" << scene_;
	lo_message m=lo_message_new();
	lo_message_add(m,"i",scene_);
	return m;
}

// /live/undo
live_undo_message_t::live_undo_message_t()
{
}
const char *live_undo_message_t::topic() { return "/live/undo";}
lo_message live_undo_message_t::createMessage()
{
    // pic::logmsg() << "live_undo_message_t::createMessage() " << topic() ;
	lo_message m=lo_message_new();
	return m;
}


// /live/redo
live_redo_message_t::live_redo_message_t()
{
}
const char *live_redo_message_t::topic() { return "/live/redo";}
lo_message live_redo_message_t::createMessage()
{
    // pic::logmsg() << "live_redo_message_t::createMessage() " << topic() ;
	lo_message m=lo_message_new();
	return m;
}





} /* namespace oscpad_plg */
