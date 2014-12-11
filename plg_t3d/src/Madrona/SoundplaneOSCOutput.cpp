
// Part of the Soundplane client software by Madrona Labs.
// Copyright (c) 2013 Madrona Labs LLC. http://www.madronalabs.com
// Distributed under the MIT license: http://madrona-labs.mit-license.org/

#include "SoundplaneOSCOutput.h"

#include <stdexcept>

#include "c99compat.h"

const char* kDefaultHostnameString = "localhost";

OSCVoice::OSCVoice() :
    startX(0),
    startY(0),
    x(0),
    y(0),
    z(0),
    z1(0),
    note(0),
    mState(kVoiceStateInactive)
{
}

OSCVoice::~OSCVoice()
{
}

// --------------------------------------------------------------------------------
#pragma mark SoundplaneOSCOutput

SoundplaneOSCOutput::SoundplaneOSCOutput() :
	mDataFreq(250.),
	mLastFrameStartTime(0),
	mpUDPSocket(0),
	mFrameId(0),
	mSerialNumber(0),
	lastInfrequentTaskTime(0),
	mKymaMode(false),
    mGotNoteChangesThisFrame(false),
    mGotMatrixThisFrame(false)
{
}

SoundplaneOSCOutput::~SoundplaneOSCOutput()
{
	if (mpOSCBuf) delete[] mpOSCBuf;
}

void SoundplaneOSCOutput::initialize()
{
    mpUDPSocket = new UdpSocket();
	mpOSCBuf = new char[kUDPOutputBufferSize];
}

bool SoundplaneOSCOutput::connect(const char* name, int port)
{
    if(mpUDPSocket)
	{
		try
		{
			// this will disconnect socket
			delete mpUDPSocket;
		    mpUDPSocket = new UdpSocket();

		    mpUDPSocket->Connect(IpEndpointName(name, port));
			return true;
		}
		catch(std::runtime_error& err)
		{
			//MLError() << "SoundplaneOSCOutput: error connecting to " << name << ", port " << port << "\n";
		}
		//debug() << "SoundplaneOSCOutput:connected to " << name << ", port " << port << "\n";

	}
    return false;
}

int SoundplaneOSCOutput::getKymaMode()
{
	return mKymaMode;
}

void SoundplaneOSCOutput::setKymaMode(bool m)
{
	mKymaMode = m;
}

void SoundplaneOSCOutput::setActive(bool v)
{
	mActive = v;
	
	// reset frame ID
	mFrameId = 0;
}

void SoundplaneOSCOutput::modelStateChanged()
{

}

void SoundplaneOSCOutput::doInfrequentTasks()
{
	if(!mpUDPSocket) return;
	if(!mpOSCBuf) return;
	osc::OutboundPacketStream p( mpOSCBuf, kUDPOutputBufferSize );
	if(mKymaMode)
	{
		p << osc::BeginBundleImmediate;
		p << osc::BeginMessage( "/osc/respond_to" );	
		p << (osc::int32)kDefaultUDPReceivePort;
		p << osc::EndMessage;
		
		p << osc::BeginMessage( "/osc/notify/midi/Soundplane" );	
		p << (osc::int32)1;
		p << osc::EndMessage;

		p << osc::BeginMessage( "/t3d/dr" );
		p << (osc::int32)mDataFreq;
		p << osc::EndMessage;

		p << osc::EndBundle;
		mpUDPSocket->Send( p.Data(), p.Size() );
		return;
	}
	
	// send data rate to receiver
	p << osc::BeginBundleImmediate;
	p << osc::BeginMessage( "/t3d/dr" );
	p << (osc::int32)mDataFreq;
	p << osc::EndMessage;
	p << osc::EndBundle;
	mpUDPSocket->Send( p.Data(), p.Size() );
}

// unused?
void SoundplaneOSCOutput::notify(int connected)
{
	if(!mpUDPSocket) return;
	if(!mpOSCBuf) return;
	if(!mActive) return;
	osc::OutboundPacketStream p( mpOSCBuf, kUDPOutputBufferSize );
	p << osc::BeginBundleImmediate;
	p << osc::BeginMessage( "/t3d/con" );	
	p << (osc::int32)connected;
	p << osc::EndMessage;
	p << osc::EndBundle;
	mpUDPSocket->Send( p.Data(), p.Size() );
	//debug() << "SoundplaneOSCOutput::notify\n";
}

void SoundplaneOSCOutput::processMessage(const SoundplaneDataMessage* msg)
{
    static const MLSymbol startFrameSym=MLS_startFrameSym;
    static const MLSymbol touchSym=MLS_touchSym;
    static const MLSymbol onSym=MLS_onSym;
    static const MLSymbol continueSym=MLS_continueSym;
    static const MLSymbol offSym=MLS_offSym;
    static const MLSymbol controllerSym=MLS_controllerSym;
    static const MLSymbol xSym=MLS_xSym;
    static const MLSymbol ySym=MLS_ySym;
    static const MLSymbol xySym=MLS_xySym;
    static const MLSymbol zSym=MLS_zSym;
    static const MLSymbol toggleSym=MLS_toggleSym;
    static const MLSymbol endFrameSym=MLS_endFrameSym;
    static const MLSymbol matrixSym=MLS_matrixSym;
    static const MLSymbol nullSym=MLS_nullSym;
    
	if (!mActive) return;
    MLSymbol type = msg->mType;
    MLSymbol subtype = msg->mSubtype;
    
    int i;
	float x, y, z, dz, note;
    
    if(type == startFrameSym)
    {
        const osc::uint64 dataPeriodMicrosecs = 1000*1000 / mDataFreq;
        mCurrFrameStartTime = getMicroseconds();
        if (mCurrFrameStartTime > mLastFrameStartTime + (osc::uint64)dataPeriodMicrosecs)
        {
            mLastFrameStartTime = mCurrFrameStartTime;
            mTimeToSendNewFrame = true;
        }
        else
        {
            mTimeToSendNewFrame = false;
        }        
        mGotNoteChangesThisFrame = false;
        mGotMatrixThisFrame = false;
    }
    else if(type == touchSym)
    {
        // get touch data
        i = msg->mData[0];
        x = msg->mData[1];
        y = msg->mData[2];
        z = msg->mData[3];
        dz = msg->mData[4];
        note = msg->mData[5];
        OSCVoice* pVoice = &mOSCVoices[i];        
        pVoice->x = x;
        pVoice->y = y;
        pVoice->z1 = pVoice->z;
        pVoice->z = z;
        pVoice->note = note;
        
        if(subtype == onSym)
        {
            pVoice->startX = x;
            pVoice->startY = y;
            pVoice->mState = kVoiceStateOn;
            mGotNoteChangesThisFrame = true;
        }
        if(subtype == continueSym)
        {
            pVoice->mState = kVoiceStateActive;
        }
        if(subtype == offSym)
        {
            if((pVoice->mState == kVoiceStateActive) || (pVoice->mState == kVoiceStateOn))
            {
                pVoice->mState = kVoiceStateOff;
                pVoice->z = 0;
                mGotNoteChangesThisFrame = true;
            }
        }
    }
    else if(type == controllerSym)
    {
        // when a controller message comes in, make a local copy of the message and store by zone ID.
        int zoneID = msg->mData[0];
        mMessagesByZone[zoneID] = *msg;
    }
    else if(type == matrixSym)
    {
        // store matrix to send with bundle
        mGotMatrixThisFrame = true;
        mMatrixMessage = *msg;
    }
    else if(type == endFrameSym)
    {
        if(mGotNoteChangesThisFrame || mTimeToSendNewFrame)
        {
            // begin OSC bundle for this frame
            osc::OutboundPacketStream p( mpOSCBuf, kUDPOutputBufferSize );
            
            // timestamp is now stored in the bundle, synchronizing all info for this frame.
            p << osc::BeginBundle(mCurrFrameStartTime);
            
			// send frame message
			// /k1/frm frameID serialNumber
			//
			p << osc::BeginMessage( "/t3d/frm" );
			p << mFrameId++ << mSerialNumber;
			p << osc::EndMessage;
            
            // for each zone, send and clear any controller messages received since last frame
            for(int i=0; i<kSoundplaneAMaxZones; ++i)
            {
                SoundplaneDataMessage* pMsg = &(mMessagesByZone[i]);
                if(pMsg->mType == controllerSym)
                {
                    // send controller message: /t3d/[zoneName] val1 (val2)
                    // TODO allow zones to split touches and controls across different ports
                    // using the channel attribute. (channel = port number offset for OSC)
                    // int channel = pMsg->mData[1];
                    // int ctrlNum1 = pMsg->mData[2];
                    // int ctrlNum2 = pMsg->mData[3];
                    // int ctrlNum3 = pMsg->mData[4];
                    x = pMsg->mData[5];
                    y = pMsg->mData[6];
                    z = pMsg->mData[7];
                    std::string ctrlStr("/t3d/");
                    ctrlStr += (pMsg->mZoneName);

                    p << osc::BeginMessage( ctrlStr.c_str() );

                    // get control data by type and add to message
                    if(pMsg->mSubtype == xSym)
                    {
                        p << x;
                    }
                    else if(pMsg->mSubtype == ySym)
                    {
                        p << y;
                    }
                    else if (pMsg->mSubtype == xySym)
                    {
                        p << x << y;
                    }
                    else if (pMsg->mSubtype == zSym)
                    {
                        p << z;
                    }
                    else if (pMsg->mSubtype == toggleSym)
                    {
                        int t = (x > 0.5f);
                        p << t;
                    }
                    p << osc::EndMessage;

                    // clear
                    mMessagesByZone[i].mType = nullSym;
                }
            }
            // send notes, either in Kyma mode, or not
            if (!mKymaMode)
            {
                // send 1 message for each live touch: /t3d/tch[touchID] x y z note
                // age is not sent over OSC-- to be reconstructed on the receiving end if needed.
                //
                for(int i=0; i<mMaxTouches; ++i)
                {
                    OSCVoice* pVoice = &mOSCVoices[i];
                    if(pVoice->mState != kVoiceStateInactive)
                    {
                        osc::int32 touchID = i + 1; // 1-based for OSC
                        std::string address("/t3d/tch");
                        const int maxSize = 4;
                        char idBuf[maxSize];
                        snprintf(idBuf, maxSize, "%d", touchID);                                 
                        address += std::string(idBuf);
                        p << osc::BeginMessage( address.c_str() );
                        p << pVoice->x << pVoice->y << pVoice->z << pVoice->note;
                        p << osc::EndMessage;
                    }
                    
                    if (pVoice->mState == kVoiceStateOff)
                    {
                        pVoice->mState = kVoiceStateInactive;
                    }
                }
               
            }
            else // kyma
            {
                for(int i=0; i<mMaxTouches; ++i)
                {
                    OSCVoice* pVoice = &mOSCVoices[i];			
                    osc::int32 touchID = i; // 0-based for Kyma
                    osc::int32 offOn = 1;
                    if(pVoice->mState == kVoiceStateOn)
                    {
                        offOn = -1;
                    }
                    else if (pVoice->mState == kVoiceStateOff)
                    {
                        offOn = 0; // TODO periodically turn off silent voices 
                    }
                
                    if(pVoice->mState != kVoiceStateInactive)
                    {
                        p << osc::BeginMessage( "/key" );	
                        p << touchID << offOn << pVoice->note << pVoice->z << pVoice->y;
                        p << osc::EndMessage;
                    }
                }
            }
            
            // format and send matrix in OSC blob if we got one
            if(mGotMatrixThisFrame)
            {
                p << osc::BeginMessage( "/t3d/matrix" );
                p << osc::Blob( &(msg->mMatrix), sizeof(msg->mMatrix) );
                p << osc::EndMessage;
                mGotMatrixThisFrame = false;
            }
            
            // end OSC bundle and send
            p << osc::EndBundle;
            mpUDPSocket->Send( p.Data(), p.Size() );
        }
    }
}

