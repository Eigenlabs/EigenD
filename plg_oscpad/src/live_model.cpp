/*
 * live_model.cpp
 *
 *  Created on: Jul 1, 2013
 *      Author: kodiak
 */

#include "live_model.h"

namespace oscpad_plg {

live_model_t::live_model_t() : current_recording_clip_(NULL), armedTrack_(-1)
{
}

live_model_t::~live_model_t() {
	std::map<std::pair<int,int>,clip_t* >::iterator c;
	for(c=clips_.begin();c!=clips_.end();c++)
	{
		delete c->second;
	}
}

std::list<live_model_t::clip_t*> live_model_t::clips()
{
	std::map<std::pair<int,int>,clip_t* >::iterator c;
	std::list<live_model_t::clip_t*> ret;
	for(c=clips_.begin();c!=clips_.end();c++)
	{
		ret.push_back(c->second);
	}
	return ret;
}


void live_model_t::changeState(int track, int clip, clip_state_t state)
{
	std::pair<int,int> location = std::make_pair(track,clip);
	std::map<std::pair<int,int>,clip_t* >::iterator c=clips_.find(location);
	if(c==clips_.end())
	{
		// new state
		clip_t *new_clip=new clip_t(track,clip,state);
		clips_.insert(std::make_pair(location,new_clip));
	}
	else
	{
		c->second->changeState(state);
	}
}


} /* namespace oscpad_plg */
