/*
 * live_model.cpp
 *
 *  Created on: Jul 1, 2013
 *      Author: Mark Harris
 */

#include "live_model.h"

namespace live_model {

live_model_t::live_model_t() : current_recording_clip_(NULL), armedTrack_(-1)
{
}

live_model_t::~live_model_t()
{
	clear();
}

std::list<clip_t*> live_model_t::clips()
{
	std::map<std::pair<int,int>,clip_t* >::iterator c;
	std::list<clip_t*> ret;
	for(c=clips_.begin();c!=clips_.end();c++)
	{
		ret.push_back(c->second);
	}
	return ret;
}
clip_t::state_t  live_model_t::getState(int track, int clip)
{
	std::pair<int,int> location = std::make_pair(track,clip);
	std::map<std::pair<int,int>,clip_t* >::iterator c=clips_.find(location);
	if(c==clips_.end())
	{
		return clip_t::CLIP_EMPTY;
	}
	return c->second->state_;
}


void live_model_t::changeState(int track, int clip, clip_t::state_t state)
{
	std::pair<int,int> location = std::make_pair(track,clip);
	std::map<std::pair<int,int>,clip_t* >::iterator c=clips_.find(location);
	if(c==clips_.end())
	{
		// new state
		clip_t *new_clip=new clip_t(track,clip,state);
		clips_.insert(std::make_pair(location,new_clip));
		if(state==clip_t::CLIP_RECORDING)
		{
			current_recording_clip_=new_clip;
		}
	}
	else
	{
		c->second->changeState(state);
		if(state==clip_t::CLIP_RECORDING)
		{
			current_recording_clip_=c->second;
		}
	}
}

void live_model_t::clear()
{
	current_recording_clip_=NULL;
	armedTrack_=-1;
	std::map<std::pair<int,int>,clip_t* >::iterator c;
	for(c=clips_.begin();c!=clips_.end();c++)
	{
		delete c->second;
	}
	clips_.clear();
}


std::list<clip_t*> live_view_t::clips()
{
	std::map<std::pair<int,int>,clip_t* >::iterator c;
	std::list<clip_t*> ret;
	for(c=model_.clips_.begin();c!=model_.clips_.end();c++)
	{
		if(c->second->clip_>= (int) top_ && c->second->clip_< (int) (top_+height_ )
				&& c->second->track_>=(int) left_ && c->second->track_< (int) (left_+width_) )
		{
			ret.push_back(c->second);
		}
	}
	return ret;
}



} /* namespace oscpad_plg */
