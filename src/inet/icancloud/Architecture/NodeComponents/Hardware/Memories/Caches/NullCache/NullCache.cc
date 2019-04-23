#include "NullCache.h"

namespace inet {

namespace icancloud {


Define_Module (NullCache);


NullCache::~NullCache(){
}


void NullCache::initialize(int stage) {
    icancloud_Base::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        std::ostringstream osStream;
        int i;

        // Set the moduleIdName
        osStream << "NullCache." << getId();
        moduleIdName = osStream.str();

        // Module parameters
        numInputs = par("numInputs");

        // Init the gate IDs to/from Input gates...
        toInputGates = new cGate*[numInputs];
        fromInputGates = new cGate*[numInputs];

        for (i = 0; i < numInputs; i++) {
            toInputGates[i] = gate("toInput", i);
            fromInputGates[i] = gate("fromInput", i);
        }

        // Init the gate IDs to/from Output
        toOutputGate = gate("toOutput");
        fromOutputGate = gate("fromOutput");
    }
}


void NullCache::finish(){

	// Finish the super-class
	icancloud_Base::finish();
}


cGate* NullCache::getOutGate (cMessage *msg){

	int i;

		// If msg arrive from Output
		if (msg->getArrivalGate()==fromOutputGate){
			if (gate("toOutput")->getNextGate()->isConnected()){
				return (toOutputGate);
			}
		}

		// If msg arrive from Inputs
		else if (msg->arrivedOn("fromInput")){
			for (i=0; i<numInputs; i++)
				if (msg->arrivedOn ("fromInput", i))
					return (gate("toInput", i));
		}

	// If gate not found!
	return nullptr;
}


void NullCache::processSelfMessage (cMessage *msg){
	showErrorMessage ("Unknown self message [%s]", msg->getName());
}


void NullCache::processRequestMessage (Packet *pkt){

    const auto &sm = pkt->peekAtFront<icancloud_Message>();
    if (sm == nullptr)
        throw cRuntimeError("packet doesn't include icancloud_Message header");
    sendRequestMessage (pkt, toOutputGate);
}


void NullCache::processResponseMessage (Packet *pkt){
    const auto &sm = pkt->peekAtFront<icancloud_Message>();
    if (sm == nullptr)
        throw cRuntimeError("packet doesn't include icancloud_Message header");
	sendResponseMessage (pkt);
}


} // namespace icancloud
} // namespace inet
