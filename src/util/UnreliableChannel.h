//-------------------------------------------------------------------------------
//      UnreliableChannel.h --
//
//      This file declares "UnreliableChannel" class for RDT protocols.
//
//      Copyright (C) 2010 Kyeong Soo (Joseph) Kim
//-------------------------------------------------------------------------------


#ifndef __UNRELIABLE_CHANNEL_H
#define __UNRELIABLE_CHANNEL_H

#include <omnetpp.h>

// Unreliable channel where packets may experience bit errors (resulting in packet error)
// and packet losses.
class UnreliableChannel: public cSimpleModule {
private:
	double datarate;
	double delay; // propagation delay in one direction (not round-trip delay)
	double per; // packet error rate in [0, 1]
	double plr; // packet loss rate in [0, 1]
	int leftGateId; // gate ID of (input part of) leftGate
//	int rightGateId; // gate ID of (input part of) rightGate

protected:
	virtual void initialize();
	virtual void handleMessage(cMessage *msg);
	virtual void finish();
};

#endif
