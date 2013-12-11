/* -*- mode:c++ -*- ********************************************************
 * file:        NotConnectedRNodePhyLayer.h
 *
 * author:      Karl Wessel
 ***************************************************************************
 * part of:     mixim framework
 * description: application layer which expects to receive and answers at
				least one broadcast
 ***************************************************************************/


#ifndef NOT_CONNECTED_RNODE_PHY_LAYER_H
#define NOT_CONNECTED_RNODE_PHY_LAYER_H

#include "CMPhyLayer.h"

class NotConnectedRNodePhyLayer : public CMPhyLayer
{
public:
	bool broadcastReceived;

	NotConnectedRNodePhyLayer()
		: CMPhyLayer()
		, broadcastReceived(false)
	{}

	virtual void initialize(int stage) {
		CMPhyLayer::initialize(stage);
		if(stage==0){
			broadcastReceived = false;
		}
	}

	virtual void finish() {
		cComponent::finish();

		assertFalse("Should have received no broadcast.", broadcastReceived);
	}

protected:
	virtual void handleLowerMsg( const MACAddress& srcAddr) {
		EV << "Not Connected R-Node " << findContainingNode(this)->getIndex() << ": got broadcast message from " << srcAddr << endl;

		broadcastReceived = true;
	}
};

#endif

