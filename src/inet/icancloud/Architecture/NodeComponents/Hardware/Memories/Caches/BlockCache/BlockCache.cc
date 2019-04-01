#include "BlockCache.h"

namespace inet {

namespace icancloud {


Define_Module (BlockCache);


BlockCache::~BlockCache(){
}


void BlockCache::initialize(int stage) {
    icancloud_Base::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        std::ostringstream osStream;

        // Set the moduleIdName
        osStream << "BlockCache." << getId();
        moduleIdName = osStream.str();

        // Init the super-class

        // Module parameters
        numInputs = par("numInputs");
        hitRatio = par("hitRatio");

        // Check the Volume Manager stride size
        if ((hitRatio < 0) || (hitRatio > 1))
            showErrorMessage(
                    "Invalid hitRatio value [%f]. Must be (0 <= hitRatio >= 1)",
                    hitRatio);

        // Init the gate IDs
        toInputGate = gate("toInput");
        fromInputGate = gate("fromInput");
        toOutputGate = gate("toOutput");
        fromOutputGate = gate("fromOutput");
    }
}


void BlockCache::finish(){

	// Finish the super-class
	icancloud_Base::finish();
}


cGate* BlockCache::getOutGate (cMessage *msg){

    // If msg arrive from Output
    if (msg->getArrivalGate()==fromOutputGate){
        if (gate("toOutput")->getNextGate()->isConnected()){
            return (toOutputGate);
        }
    }

    // If msg arrive from Input
    else if (msg->getArrivalGate()==fromInputGate){
        if (gate("toInput")->getNextGate()->isConnected()){
            return (toInputGate);
        }
    }


	// If gate not found!
	return nullptr;
}


void BlockCache::processSelfMessage (cMessage *msg){
	showErrorMessage ("Unknown self message [%s]", msg->getName());
}


void BlockCache::processRequestMessage (Packet  *pkt){
	sendRequestMessage (pkt, toOutputGate);
}


void BlockCache::processResponseMessage (Packet  *pkt){
	sendResponseMessage (pkt);
}


} // namespace icancloud
} // namespace inet
