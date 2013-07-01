#ifndef LIVE_OSC_H_
#define LIVE_OSC_H_

#include "osc_server.h"
#include "osc_client.h"

namespace oscpad_plg {

struct live_handler_t : public osc_handler_t
{
public:
	virtual ~live_handler_t() {};
};

struct live_message_t : public osc_message_t
{
public:
	virtual ~live_message_t() {};
};

// messages defined here: http://monome.q3f.org/browser/trunk/LiveOSC/OSCAPI.txt   , note: some errors, check src for definitive

// /live/beat (beat number)
struct live_beat_handler_t :public live_handler_t
{
	virtual const char *topic();
    virtual void messageHandler(const char *path,lo_message& msg);
    virtual ~live_beat_handler_t() {} ;

	virtual void processBeat(int beat) = 0;
};

// /live/clip/info  (track, clip, state)
struct live_clip_info_handler_t :public live_handler_t
{
	enum state_t
	{
		EMPTY=0,
		HAS_CLIP,
		PLAYING,
		TRIGGERED
	};

	virtual const char *topic();
    virtual void messageHandler(const char *path,lo_message& msg);
    virtual ~live_clip_info_handler_t() {} ;

	virtual void processClip(int track,int clip,state_t state)= 0;
};

// /live/track/info iiiif 0 1 0 2 63072000.000000, track, armed, clip, playing, length
struct live_track_info_handler_t :public live_handler_t
{
	enum state_t
	{
		EMPTY=0,
		HAS_CLIP,
		PLAYING,
		TRIGGERED
	};

	virtual const char *topic();
    virtual void messageHandler(const char *path,lo_message& msg);
    virtual ~live_track_info_handler_t() {} ;

	virtual void processClip(int track,bool armed, int clip, state_t state, float length)= 0;
};



// /live/arm (track,enable)
struct live_arm_message_t : public live_message_t
{
	live_arm_message_t(int track, bool armed);
	virtual const char *topic();
	virtual lo_message createMessage();
	int track_;
	bool armed_;
};

// /live/clip/info (track,clip)
struct live_clip_info_message_t : public live_message_t
{
	live_clip_info_message_t(int track,int clip);
	virtual const char *topic();
	virtual lo_message createMessage();
	int track_;
	int clip_;
};



// /live/play/clipslot (track,clip)
struct live_clip_slot_message_t : public live_message_t
{
	live_clip_slot_message_t(int track, int clip);
	virtual const char *topic();
	virtual lo_message createMessage();
	int track_;
	int clip_;
};


} /* namespace oscpad_plg */
#endif /* LIVE_OSC_H_ */
