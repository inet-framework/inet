/* -*- mode:c++ -*- ********************************************************
 * file:        NotConnectedBCNodePhyLayer.h
 *
 * author:      Karl Wessel
 ***************************************************************************
 * part of:     mixim framework
 * description: physical layer which broadcasts packets and expects no
				answer
 ***************************************************************************/


#ifndef NOT_CONNECTED_BCNODE_PHY_LAYER_H
#define NOT_CONNECTED_BCNODE_PHY_LAYER_H

#include "CMPhyLayer.h"

using std::endl;

class NotConnectedBCNodePhyLayer : public CMPhyLayer
{
public:
    //Module_Class_Members(NotConnectedBCNodePhyLayer, CMPhyLayer, 0);

	bool broadcastAnswered;

	NotConnectedBCNodePhyLayer()
		: CMPhyLayer()
		, broadcastAnswered(false)
	{}

	virtual void initialize(int stage) {
		CMPhyLayer::initialize(stage);
		if(stage==0){
			broadcastAnswered = false;
			scheduleAt(simTime() + 1.0 + (static_cast<double>(findContainingNode(this)->getIndex())) * 0.1, new cMessage(0,10));
		}
	}

	virtual void finish() {
		cComponent::finish();

		assertFalse("Broadcast should not be answered by any node.",
					broadcastAnswered);
	}
protected:
	virtual void handleLowerMsg(const MACAddress& srcAddr) {
		broadcastAnswered = true;
		EV << "Not Connected BC-Node " << findContainingNode(this)->getIndex() << ": got answer message from " << srcAddr << endl;
	}

	virtual void handleSelfMsg() {
		// we should send a broadcast packet ...
		EV << "Not Connected BC-Node " << findContainingNode(this)->getIndex() << ": Sending broadcast packet!" << endl;
		sendDown(MACAddress::BROADCAST_ADDRESS);
	}
};

#endif

