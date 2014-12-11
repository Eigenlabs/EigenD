
// Part of the Soundplane client software by Madrona Labs.
// Copyright (c) 2013 Madrona Labs LLC. http://www.madronalabs.com
// Distributed under the MIT license: http://madrona-labs.mit-license.org/

#ifndef __SOUNDPLANE_OSC_OUTPUT_H_
#define __SOUNDPLANE_OSC_OUTPUT_H_

#include <vector>
#include <list>

#include <lib_op/osc/OscOutboundPacketStream.h>
#include <lib_op/ip/UdpSocket.h>
#include "Madrona.h"

extern const char* kDefaultHostnameString;
const int kDefaultUDPPort = 3123;
const int kDefaultUDPReceivePort = 3124;
const int kUDPOutputBufferSize = 4096;



class OSCVoice
{
public:
	OSCVoice();
	~OSCVoice();

	float startX;
	float startY;
    float x;
    float y;
    float z;
    float z1;
    float note;
	VoiceState mState;
};

class SoundplaneOSCOutput :
	public SoundplaneDataListener
{
public:
	SoundplaneOSCOutput();
	~SoundplaneOSCOutput();
	void initialize();
	
	bool connect(const char* name, int port);
	int getKymaMode();
	void setKymaMode(bool m);
	
    // SoundplaneDataListener
    void processMessage(const SoundplaneDataMessage* msg);
    
	void modelStateChanged();
	void setDataFreq(float f) { mDataFreq = f; }
	float getDataFreq() { return mDataFreq;}
	
	void setActive(bool v);
	void setMaxTouches(int t) { mMaxTouches = clamp(t, 0, kSoundplaneMaxTouches); }
	int getMaxTouches() { return mMaxTouches;}
	
	void setSerialNumber(int s) { mSerialNumber = s; }
	void notify(int connected);
	
	void doInfrequentTasks();

private:	

	int mMaxTouches;	
	OSCVoice mOSCVoices[kSoundplaneMaxTouches];
    SoundplaneDataMessage mMessagesByZone[kSoundplaneAMaxZones];
    
	float mDataFreq;
	osc::uint64 mCurrFrameStartTime;
	osc::uint64 mLastFrameStartTime;
    bool mTimeToSendNewFrame;

	UdpSocket* mpUDPSocket;
    char* mpOSCBuf;
	osc::int32 mFrameId;
	int mSerialNumber;
	
	osc::uint64 lastInfrequentTaskTime;
	bool mKymaMode;
    bool mGotNoteChangesThisFrame;
    bool mGotMatrixThisFrame;
    SoundplaneDataMessage mMatrixMessage;
};


#endif // __SOUNDPLANE_OSC_OUTPUT_H_
