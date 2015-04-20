//-------------------------------------------------------------------------------
//      SGMChannel.h --
//
//      This file declares "SGMChannel" class.
//
//      Copyright (C) 2015 Kyeong Soo (Joseph) Kim
//-------------------------------------------------------------------------------


#ifndef __SGM_CHANNEL_H
#define __SGM_CHANNEL_H

#include <omnetpp.h>

// SGM for lossy channel.
class SGMChannel: public cSimpleModule {
private:
    enum SGM_STATE {GOOD, BAD};
    SGM_STATE state;    // SGM state
	double datarate;
	double delay; // propagation delay in one direction (not round-trip delay)
	double p; // transition probability from GOOD to BAD state
	double q; // transition probability from BAD to GOOD state
	int leftGateId; // gate ID of (input part of) leftGate
//	int rightGateId; // gate ID of (input part of) rightGate

protected:
	virtual void initialize();
	virtual void handleMessage(cMessage *msg);
	virtual void finish();
};

#endif
