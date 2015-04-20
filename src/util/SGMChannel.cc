//-------------------------------------------------------------------------------
//      SGMChannel.cc --
//
//      This file implements "SGMChannel" class.
//
//      Copyright (C) 2015 Kyeong Soo (Joseph) Kim
//-------------------------------------------------------------------------------


#include "SGMChannel.h"

Define_Module( SGMChannel);

void SGMChannel::initialize() {
	// initialize member variables to the values of module parameters.
    state = GOOD;   // we start in GOOD state
	datarate = par("datarate");
	delay = par("delay");
	p = par("p");
	q = par("q");
	leftGateId = gate("leftGate$i")->getId();
//	rightGate = gate("rightGate$i")->getId();
}

void SGMChannel::handleMessage(cMessage *msg) {
	EV << "Received " << msg->getName() << endl;

	// make sure that all received messages are packets.
	cPacket *pkt = check_and_cast<cPacket *> (msg);

	if (state == GOOD) {
	    if (dblrand() <= p) {
	        state = BAD;
	    }
	} else {
	    if (dblrand() <= q)
	        state = GOOD;
	}

	if (state == BAD) {
		// drop the packet.
		delete pkt;
	} else {
		if (pkt->getArrivalGateId() == leftGateId) {
			sendDelayed(pkt, delay + pkt->getBitLength()/datarate, "rightGate$o");
		} else {
			sendDelayed(pkt, delay + pkt->getBitLength()/datarate, "leftGate$o");
		}
	}
}

void SGMChannel::finish() {
}
