/*
 * live_model.h
 *
 *  Created on: Jul 1, 2013
 *      Author: kodiak
 */

#ifndef LIVE_MODEL_H_
#define LIVE_MODEL_H_

#include <map>
#include <list>

namespace live_model {
// this model represents the state of each clip and track status

struct clip_t
{
	enum state_t
	{
		CLIP_EMPTY=0,
		CLIP_HAS_CLIP,
		CLIP_PLAYING,
		CLIP_TRIGGERED,
		CLIP_RECORDING
	};

	int track_;
	int clip_;
	state_t state_;

	clip_t(int track, int clip, state_t state) : track_(track), clip_(clip), state_(state) { };

	void changeState(state_t state) { state_=state; }
};


//	enum track_state_t
//	{
//		TRACK_UNARMED=0,
//		TRACK_ARMED
//	};

class live_model_t
{
	friend class live_view_t;
public:



	live_model_t();
	virtual ~live_model_t();
protected:
	clip_t::state_t getState(int track, int clip);
	void changeState(int track, int clip, clip_t::state_t state);
	bool isRecording() { return current_recording_clip_ != 0ULL; }
	clip_t* getRecordingClip() { return current_recording_clip_;}
	int armTrack(int track) { int prevT=armedTrack_; armedTrack_=track; return prevT;}
	void clear();

	std::list<clip_t*> clips();


private:
	std::map<std::pair<int,int>,clip_t*> clips_;
	clip_t *current_recording_clip_;
	int armedTrack_;

	// prevent copying
	live_model_t(const live_model_t&);
	live_model_t operator=(const live_model_t&);

};

class live_view_t
{
public:
	live_view_t(live_model_t& m, unsigned top, unsigned height, unsigned left, unsigned width ): model_(m), top_(top), height_(height), left_(left), width_(width) {};

	live_model_t &model() { return model_;}
	clip_t::state_t getState(int track, int clip) { return model_.getState(track,clip);}
	void changeState(int track, int clip, clip_t::state_t state) { model_.changeState(track,clip, state);}
	bool isRecording() { return model_.isRecording(); }
	clip_t* getRecordingClip() { return model_.getRecordingClip();}
	int armTrack(int track) { return model_.armTrack(track);}

	void setWindow(unsigned top, unsigned height, unsigned left, unsigned width)
	{
		top_=top;
	    height_=height;
		left_=left;
		width_=width;
	}
	unsigned clipOffset() { return top_;}
	unsigned trackOffset() { return left_;}
	unsigned clipSize() { return height_;}
	unsigned trackSize() { return width_;}
	void clear() { return model_.clear();}

	std::list<clip_t*> clips();

private:
	live_model_t& model_;
	unsigned top_, height_, left_, width_;

	// prevent copying
	live_view_t(const live_view_t&);
	live_view_t operator=(const live_view_t&);

};


} /* namespace livepad_plg */
#endif /* LIVE_MODEL_H_ */
