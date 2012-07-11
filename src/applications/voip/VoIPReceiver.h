/*
 * VoIPReceiver.h
 *
 *  Created on: 01/feb/2011
 *      Author: Adriano
 */



#ifndef VOIPRECEIVER_H_
#define VOIPRECEIVER_H_

#include <string.h>
#include <omnetpp.h>

#include "IPvXAddressResolver.h"
#include "UDPSocket.h"
#include "VoipPacket_m.h"
#include <list>

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

	int 	 	emodel_Ie_;
	int 	    emodel_Bpl_;
	int 		emodel_A_;
	double 		emodel_Ro_;

	typedef std::list<VoipPacket*> PacketsList;
	PacketsList  mPacketsList;
	PacketsList  mPlayoutQueue;
	unsigned int mCurrentTalkspurt;
	unsigned int mBufferSpace;
	simtime_t    mSamplingDelta;
	simtime_t    mPlayoutDelay;

	bool 		mInit;

	simsignal_t mFrameLossSignal;
	simsignal_t mFrameDelaySignal;
	simsignal_t mPlayoutDelaySignal;
	simsignal_t mPlayoutLossSignal;
	simsignal_t mMosSignal;
	simsignal_t mTaildropLossSignal;

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


