/*
 * VoIPReceiver.h
 *
 *  Created on: 01/feb/2011
 *      Author: Adriano
 */



#ifndef VOIPRECEIVER_H_
#define VOIPRECEIVER_H_

#include <string.h>
#include <list>

#include "INETDefs.h"

#include "IPvXAddressResolver.h"
#include "UDPSocket.h"
#include "VoipPacket_m.h"

class VoIPReceiver : public cSimpleModule
{
	class TaggedSample : public cObject
	{
		public:
			double sample;
			unsigned int id;
			// the emitting cComponent (module)
			cComponent* module;
	};
	UDPSocket socket;

	~VoIPReceiver();

    // FIXME: avoid _ characters
	int 	 	emodel_Ie_;
	int 	    emodel_Bpl_;
	int 		emodel_A_;
	double 		emodel_Ro_;

	typedef std::list<VoipPacket*> PacketsList;
    // FIXME: welcome to Microsoft naming conventions... mFooBar -> fooBar
	PacketsList  mPacketsList;
	PacketsList  mPlayoutQueue;
	unsigned int mCurrentTalkspurt;
	unsigned int mBufferSpace;
	simtime_t    mSamplingDelta;
	simtime_t    mPlayoutDelay;

	bool 		mInit;

	simsignal_t mFrameLossRateSignal;
	simsignal_t mFrameDelaySignal;
	simsignal_t mPlayoutDelaySignal;
	simsignal_t mPlayoutLossRateSignal;
	simsignal_t mMosSignal;
	simsignal_t mTaildropLossRateSignal;

	TaggedSample* mTaggedSample;

	virtual void finish();

protected:

	virtual int numInitStages() const {return 4;}
	void initialize(int stage);
	void handleMessage(cMessage *msg);
	double eModel (double delay, double loss);
	void playout(bool finish);


};



#endif /* VOIPRECEIVER_H_ */


