/* -*- mode:c++ -*- ********************************************************
 * file:        ConnectedBCNodePhyLayer.h
 *
 * author:      Karl Wessel
 ***************************************************************************
 * part of:     mixim framework
 * description: physical test layer which broadcasts packets and expects at least
				one answer
 ***************************************************************************/


#ifndef CONNECTED_BCNODE_PHY_LAYER_H
#define CONNECTED_BCNODE_PHY_LAYER_H

#include "CMPhyLayer.h"

class ConnectedBCNodePhyLayer : public CMPhyLayer
{
public:
    //Module_Class_Members(ConnectedBCNodePhyLayer, CMPhyLayer, 0);

	bool broadcastAnswered;

	ConnectedBCNodePhyLayer()
		: CMPhyLayer()
		, broadcastAnswered(false)
	{}

	virtual void initialize(int stage) {
		CMPhyLayer::initialize(stage);
		if(stage==0){
			broadcastAnswered = false;
			scheduleAt(simTime() + 1.0, new cMessage(0,10));
		}
	}

	virtual void finish() {
		cComponent::finish();

		assertTrue("Broadcast should be answered by at least one node.",
					broadcastAnswered);
	}
protected:
	virtual void handleLowerMsg(const MACAddress& srcAddr) {
		broadcastAnswered = true;
		EV << "Connected BC-Node " << findContainingNode(this)->getIndex() << ": got answer message from " << srcAddr << endl;
	}

	virtual void handleSelfMsg() {
		// we should send a broadcast packet ...
		EV << "Connected BC-Node " << findContainingNode(this)->getIndex() << ": Sending broadcast packet!" << endl;
		sendDown(MACAddress::BROADCAST_ADDRESS);
	}
};

#endif

