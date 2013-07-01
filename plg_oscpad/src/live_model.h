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

namespace oscpad_plg {

// this model represents the state of each clip and track status
class live_model_t
{
public:
	enum clip_state_t
	{
		CLIP_EMPTY=0,
		CLIP_HAS_CLIP,
		CLIP_PLAYING,
		CLIP_TRIGGERED,
		CLIP_RECORDING
	};
//	enum track_state_t
//	{
//		TRACK_UNARMED=0,
//		TRACK_ARMED
//	};
	struct clip_t
	{

		int track_;
		int clip_;
		clip_state_t state_;

		clip_t(int track, int clip, clip_state_t state) : track_(track), clip_(clip), state_(state) { };

		void changeState(clip_state_t state) { state_=state; }
	};

	live_model_t();
	virtual ~live_model_t();

	void changeState(int track, int clip, clip_state_t state);
	bool isRecording() { return current_recording_clip_ != NULL; }
	clip_t* getRecordingClip() { return current_recording_clip_;}
	int armTrack(int track) { int prevT=armedTrack_; armedTrack_=track; return prevT;}

	std::list<clip_t*> clips();


private:
	std::map<std::pair<int,int>,clip_t*> clips_;
	clip_t *current_recording_clip_;
	int armedTrack_;

	// prevent copying
	live_model_t(const live_model_t&);
	live_model_t operator=(const live_model_t&);

};

} /* namespace oscpad_plg */
#endif /* LIVE_MODEL_H_ */
