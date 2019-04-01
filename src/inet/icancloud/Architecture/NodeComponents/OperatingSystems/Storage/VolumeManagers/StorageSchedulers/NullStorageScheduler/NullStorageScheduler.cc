#include "NullStorageScheduler.h"

namespace inet {

namespace icancloud {


Define_Module(NullStorageScheduler);

NullStorageScheduler::~NullStorageScheduler(){
}


void NullStorageScheduler::initialize(int stage) {

    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        std::ostringstream osStream;

        // Set the moduleIdName
        osStream << "NullStorageScheduler." << getId();
        moduleIdName = osStream.str();

        // Init the gates
        toInputGate = gate("toInput");
        fromInputGate = gate("fromInput");
        toOutputGate = gate("toOutput");
        fromOutputGate = gate("fromOutput");
    }
}


void NullStorageScheduler::finish(){

	// Finish the super-class
	icancloud_Base::finish();
}


cGate* NullStorageScheduler::getOutGate (cMessage *msg){

		// If msg arrive from Input
		if (msg->getArrivalGate()==fromInputGate){
			if (toInputGate->getNextGate()->isConnected()){
				return (toInputGate);
			}
		}

		// If msg arrive from Output
		if (msg->getArrivalGate()==fromOutputGate){
			if (toOutputGate->getNextGate()->isConnected()){
				return (toOutputGate);
			}
		}

	// If gate not found!
	return nullptr;
}


void NullStorageScheduler::processSelfMessage (cMessage *msg){
	showErrorMessage ("Unknown self message [%s]", msg->getName());
}


void NullStorageScheduler::processRequestMessage (Packet * pktSm){
	sendRequestMessage (pktSm, toOutputGate);
}


void NullStorageScheduler::processResponseMessage (Packet * pktSm){
	sendResponseMessage (pktSm);
}


} // namespace icancloud
} // namespace inet
