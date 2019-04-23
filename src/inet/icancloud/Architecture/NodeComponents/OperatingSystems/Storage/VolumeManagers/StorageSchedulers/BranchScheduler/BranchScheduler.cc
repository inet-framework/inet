#include "BranchScheduler.h"

namespace inet {

namespace icancloud {


Define_Module(BranchScheduler);


BranchScheduler::~BranchScheduler(){
	
	SMS_branch->clear();
	delete (SMS_branch);	
}

void BranchScheduler::initialize(int stage) {

    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        std::ostringstream osStream;

        // Set the moduleIdName
        osStream << "BranchScheduler." << getId();
        moduleIdName = osStream.str();

        // Init the gates
        toInputGate = gate("toInput");
        fromInputGate = gate("fromInput");
        toOutputGate = gate("toOutput");
        fromOutputGate = gate("fromOutput");

        // New SMS object!
        SMS_branch = new SMS_Branch();
    }
}


void BranchScheduler::finish(){

	// Finish the super-class
	icancloud_Base::finish();
}


cGate* BranchScheduler::getOutGate (cMessage *msg){

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


void BranchScheduler::processSelfMessage (cMessage *msg){
	showErrorMessage ("Unknown self message [%s]", msg->getName());
}


void BranchScheduler::processRequestMessage(Packet *pktSm) {

    //icancloud_BlockList_Message *sm_bl;

    const auto &sm = pktSm->peekAtFront<icancloud_Message>();
    const auto &sm_bl = CHK(dynamicPtrCast<const icancloud_BlockList_Message>(sm));

    // Cast!
    // sm_bl = check_and_cast<icancloud_BlockList_Message*>(sm);

    // Split and process current request!
    SMS_branch->splitRequest(pktSm);

    // Debug
    if (DEBUG_Storage_Scheduler)
        showDebugMessage("Processing original request:%s from message:%s",
                SMS_branch->requestToString(pktSm, DEBUG_SMS_Storage_Scheduler).c_str(),
                sm_bl->contentsToString(DEBUG_BRANCHES_Storage_Scheduler,
                        DEBUG_MSG_Storage_Scheduler).c_str());

    processBranches();
}


void BranchScheduler::processResponseMessage (Packet *pktSm){
	sendResponseMessage (pktSm);
}


void BranchScheduler::processBranches() {

    //icancloud_Message *subRequest;

    Packet *subRequest;
    // Send all enqueued subRequest!
    do {
        subRequest = SMS_branch->popSubRequest();
        // There is a subRequest!
        if (subRequest != nullptr) {
            sendRequestMessage(subRequest, toOutputGate);
        }

    } while (subRequest != nullptr);
}


void BranchScheduler::sendRequestMessage (Packet *pktSm, cGate* gate){

    const auto &sm = CHK(pktSm->peekAtFront<icancloud_Message>());

	// If trace is empty, add current hostName, module and request number
	if (sm->isTraceEmpty()){
	    pktSm->trimFront();
	    auto smAux = pktSm->removeAtFront<icancloud_Message>();
	    smAux->addNodeToTrace (getHostName());
		pktSm->insertAtFront(smAux);
		updateMessageTrace (pktSm);
	}
	// Send the message!
	send (pktSm, gate);
}


} // namespace icancloud
} // namespace inet
