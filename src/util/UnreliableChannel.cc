//-------------------------------------------------------------------------------
//      UnreliableChannel.cc --
//
//      This file implements "UnreliableChannel" class for RDT protocols.
//
//      Copyright (C) 2010 Kyeong Soo (Joseph) Kim
//-------------------------------------------------------------------------------


#include "UnreliableChannel.h"

Define_Module( UnreliableChannel);

void UnreliableChannel::initialize() {
	// initialize member variables to the values of module parameters.
	datarate = par("datarate");
	delay = par("delay");
	per = par("per");
	plr = par("plr");
	leftGateId = gate("leftGate$i")->getId();
//	rightGate = gate("rightGate$i")->getId();
}

void UnreliableChannel::handleMessage(cMessage *msg) {
	EV << "Received " << msg->getName() << endl;

	// make sure that all received messages are packets.
	cPacket *pkt = check_and_cast<cPacket *> (msg);

	if (dblrand() <= plr) {
		// drop the packet.
		delete pkt;
	} else {
		if (dblrand() <= per) {
			// set error flag.
			pkt->setBitError(true);
		}
		if (pkt->getArrivalGateId() == leftGateId) {
			sendDelayed(pkt, delay + pkt->getBitLength()/datarate, "rightGate$o");
		} else {
			sendDelayed(pkt, delay + pkt->getBitLength()/datarate, "leftGate$o");
		}
	}
}

void UnreliableChannel::finish() {
}
