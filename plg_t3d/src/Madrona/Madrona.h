#ifndef __MADRONA_H__
#define __MADRONA_H__


#include <string>

#include <picross/pic_time.h>

const int kSoundplaneWidth = 64;
const int kSoundplaneHeight = 8;



const int kSoundplaneMaxTouches = 16;
const int kSoundplaneAMaxZones = 10;
const int kSoundplaneMaxControllerNumber = 127;

inline osc::uint64 getMicroseconds() { return pic_microtime();}


typedef int MLSymbol;

static const MLSymbol MLS_nullSym=-1;
static const MLSymbol MLS_startFrameSym=0;
static const MLSymbol MLS_touchSym=1;
static const MLSymbol MLS_onSym=2;
static const MLSymbol MLS_continueSym=3;
static const MLSymbol MLS_offSym=4;
static const MLSymbol MLS_controllerSym=5;
static const MLSymbol MLS_xSym=6;
static const MLSymbol MLS_ySym=7;
static const MLSymbol MLS_xySym=8;
static const MLSymbol MLS_zSym=9;
static const MLSymbol MLS_toggleSym=10;
static const MLSymbol MLS_endFrameSym=11;
static const MLSymbol MLS_matrixSym=12;

static const std::string nullZone;



template <class c>
inline c (clamp)(const c& x, const c& min, const c& max)
{
	return (x < min) ? min : (x > max ? max : x);
}


enum VoiceState
{
    kVoiceStateInactive = 0,
    kVoiceStateOn,
    kVoiceStateActive,
    kVoiceStateOff
};

struct SoundplaneDataMessage
{
    MLSymbol mType;
    MLSymbol mSubtype;
    const char* mZoneName;
    float mData[8];
    float mMatrix[kSoundplaneWidth*kSoundplaneHeight];
    SoundplaneDataMessage() : mType(MLS_nullSym),mSubtype(MLS_nullSym),mZoneName(nullZone.c_str())
    {
    }
};

class SoundplaneDataListener 
{
public:
	SoundplaneDataListener() : mActive(false) {}
	virtual ~SoundplaneDataListener() {}
    virtual void processMessage(const SoundplaneDataMessage* message) = 0;
    bool isActive() { return mActive; }

protected:
	bool mActive;
};

#endif
