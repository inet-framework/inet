//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "inet/icancloud/Virtualization/VirtualMachines/VMManagement/VmMsgController/VmMsgController.h"

namespace inet {

namespace icancloud {


Define_Module(VmMsgController);


VmMsgController::~VmMsgController() {
	// TODO Auto-generated destructor stub
}

void VmMsgController::blockMessages (){
	migrateActive = true;
}

void VmMsgController::activateMessages(){
	migrateActive = false;
	flushPendingMessages();
}

bool VmMsgController::migrationPrepared (){
	return  ( ( pendingCPUMsg == 0 ) &&
			  ( pendingIOMsg == 0 ) &&
			  ( pendingNetMsg == 0 ) &&
			  ( pendingMemoryMsg == 0 )
			);
}

void VmMsgController::initialize(int stage) {

    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

        std::ostringstream osStream;

        migrateActive = false;
        pendingMessages.clear();
        pendingCPUMsg = 0;
        pendingNetMsg = 0;
        pendingMemoryMsg = 0;
        pendingIOMsg = 0;
        uId = -1;
        pId = -1;

        // Init state to idle!

        fromOSApps = new cGateManager(this);
        toOSApps = new cGateManager(this);
        fromApps = new cGateManager(this);
        toApps = new cGateManager(this);
    }

}

void VmMsgController::finish(){

}

void VmMsgController::processSelfMessage (cMessage *msg){
	delete (msg);
	std::ostringstream msgLine;
	msgLine << "Unknown self message [" << msg->getName() << "]";
	throw cRuntimeError(msgLine.str().c_str());
}

void VmMsgController::processRequestMessage (Packet *pkt){


	int operation;

	pkt->trim();

	auto sm = pkt->removeAtFront<icancloud_Message>();

	auto sm_net = dynamicPtrCast<icancloud_App_NET_Message>(sm);

	operation = sm->getOperation();

	if (operation == SM_STOP_AND_DOWN_VM) {

		if (!migrateActive){
			blockMessages();
			sm_net->setIsResponse(true);
			pkt->insertAtFront(sm_net);
			migration_msg = pkt;
			if (migrationPrepared()){
				notifyVMPreparedToMigrate();
			}

		}
		else {
		    pkt->insertAtFront(sm_net);
		    delete (pkt);
		}
	}

	else if (((int)sm->getOperation()) == SM_VM_ACTIVATION) {
	    pkt->insertAtFront(sm);
		finishMigration();
		delete (pkt);
	}

	else {

	    // Set the id of the message (the vm id)
	    sm->setPid(pId);
	    sm->setUid(uId);

		// Set as application id the arrival gate id (unique per job).
	    if ((sm_net != nullptr) && (sm_net->getCommId() != -1))
	        insertCommId (uId, pId, sm->getCommId(), pkt->getId());

		if (migrateActive){
	        pkt->insertAtFront(sm);
			pushMessage(pkt);
		}
		else {
			// If msg arrive from OS
			if (pkt->arrivedOn("fromOSApps")){
		        pkt->insertAtFront(sm);
			    sendRequestMessage(pkt, toApps->getGate(pkt->getArrivalGate()->getIndex()));
			}
			else if (pkt->arrivedOn("fromApps")){
				updateCounters(sm, 1);
				 sm->setCommId(pkt->getArrivalGate()->getIndex());

                if (sm_net != nullptr){
                    // If the message is a net message and the destination user is nullptr
                    if (sm_net->getVirtual_user() == -1){
                        // Filter the name of the user
                        sm_net->setVirtual_user(sm->getUid());
                    }
                    sm_net->setVirtual_destinationIP(sm_net->getDestinationIP());
                    sm_net->setVirtual_destinationPort(sm_net->getDestinationPort());
                    sm_net->setVirtual_localIP(sm_net->getLocalIP());
                    sm_net->setVirtual_localPort(sm_net->getLocalPort());
                }
                pkt->insertAtFront(sm);
				sendRequestMessage(pkt, toOSApps->getGate(sm->getCommId()));
			}

		}

	}
}

void VmMsgController::processResponseMessage (Packet *pkt){

		// If msg arrive from OS

	updateCounters(pkt, -1);

	if (migrateActive){
		pushMessage(pkt);
		if (migrationPrepared()){
			notifyVMPreparedToMigrate();
		}
	}
	else {
	    pkt->trim();
	    auto sm = pkt->removeAtFront<icancloud_Message>();
	    auto sm_net = dynamicPtrCast<icancloud_App_NET_Message>(sm);

	    pkt->insertAtFront(sm);
	    if (sm_net != nullptr){
	        updateCommId(pkt);
	    }

		sendResponseMessage(pkt);
	}
}

cGate* VmMsgController::getOutGate (cMessage *msg){

    // Define ..
    cGate* return_gate = nullptr;
    int i;
    bool found;

    // Initialize ..
    i = 0;
    found = false;

    // If msg arrive from OS
    if (msg->arrivedOn("fromOSApps")) {
        while ((i < gateCount()) && (!found)) {
            if (msg->arrivedOn("fromOSApps", i)) {
                return_gate = (gate("toOSApps", i));
                found = true;
            }
            i++;
        }
    }
    else if (msg->arrivedOn("fromApps")) {
        while ((i < gateCount()) && (!found)) {
            if (msg->arrivedOn("fromApps", i)) {
                return_gate = (gate("toApps", i));
                found = true;
            }
            i++;
        }
    }
    return return_gate;
}

void VmMsgController::notifyVMPreparedToMigrate(){
	sendResponseMessage(migration_msg);
}

void VmMsgController::finishMigration (){
	migrateActive = false;
	flushPendingMessages();
}

void VmMsgController::pushMessage(Packet *pkt){

    const auto & sm = pkt->peekAtFront<icancloud_Message>();
    if (sm == nullptr)
        throw cRuntimeError("Packet Error type");
	pendingMessages.insert(pendingMessages.end(), pkt);

}

Packet* VmMsgController::popMessage(){
	vector<Packet*>::iterator msgIt;

	msgIt = pendingMessages.begin();
	// check
	const auto & sm = (*msgIt)->peekAtFront<icancloud_Message>();
	if (sm == nullptr)
	    throw cRuntimeError("Packet Error type");

	pendingMessages.erase(msgIt);

	return (*msgIt);

}

void VmMsgController::sendPendingMessage (Packet* pkt){
    int smIndex;

    const auto & sm = pkt->peekAtFront<icancloud_Message>();
    if (sm == nullptr)
        throw cRuntimeError("Packet Error type");
    // The message is a Response message
    if (sm->getIsResponse()) {
        sendResponseMessage(pkt);
    }

    // The message is a request message
    else {
        smIndex = pkt->getArrivalGate()->getIndex();
        if (pkt->arrivedOn("fromOSApps")) {
            sendRequestMessage(pkt, toApps->getGate(smIndex));
        }
        else if (pkt->arrivedOn("fromApps")) {
            updateCounters(pkt, 1);
            sendRequestMessage(pkt, toOSApps->getGate(smIndex));
        }
    }
}

void VmMsgController::flushPendingMessages(){

	// Define ..
	// Extract all the messages and send to the destinations

	while (!pendingMessages.empty()){
		auto msgIt = pendingMessages.begin();

		sendPendingMessage((*msgIt));
		pendingMessages.erase(pendingMessages.begin());
	}

}

int VmMsgController::pendingMessagesSize(){
	return pendingMessages.size();
}

void VmMsgController::updateCounters (Packet *pkt, int quantity){


    const auto &sm = pkt->peekAtFront<icancloud_Message>();

	const auto &cpuMsg = dynamicPtrCast<const icancloud_App_CPU_Message>(sm);
	const auto & ioMsg = dynamicPtrCast<const icancloud_App_IO_Message>(sm);
	const auto & memMsg = dynamicPtrCast<const icancloud_App_MEM_Message>(sm);
	const auto & netMsg = dynamicPtrCast<const icancloud_App_NET_Message>(sm);

	if (cpuMsg != nullptr){
		pendingCPUMsg += quantity;
	}
	else if (ioMsg != nullptr){
		pendingIOMsg += quantity;
	}
	else if (memMsg != nullptr){
		pendingMemoryMsg += quantity;
	}
	else if (netMsg != nullptr){
		pendingNetMsg += quantity;
	}
}

void VmMsgController::updateCounters (Ptr<icancloud_Message> &sm, int quantity){


    const auto &cpuMsg = dynamicPtrCast<const icancloud_App_CPU_Message>(sm);
    const auto & ioMsg = dynamicPtrCast<const icancloud_App_IO_Message>(sm);
    const auto & memMsg = dynamicPtrCast<const icancloud_App_MEM_Message>(sm);
    const auto & netMsg = dynamicPtrCast<const icancloud_App_NET_Message>(sm);

    if (cpuMsg != nullptr){
        pendingCPUMsg += quantity;
    }
    else if (ioMsg != nullptr){
        pendingIOMsg += quantity;
    }
    else if (memMsg != nullptr){
        pendingMemoryMsg += quantity;
    }
    else if (netMsg != nullptr){
        pendingNetMsg += quantity;
    }
}


void VmMsgController::linkNewApplication(cModule* jobAppModule, cGate* scToApp, cGate* scFromApp){

    // Connections to App
    int idxToApps = toApps->newGate("toApps");
    toApps->connectOut(jobAppModule->gate("fromOS"), idxToApps);

    int idxFromApps = fromApps->newGate("fromApps");
    fromApps->connectIn(jobAppModule->gate("toOS"), idxFromApps);

    // Connections to SyscallManager
    int idxToOs = toOSApps->newGate("toOSApps");
    toOSApps->connectOut(scFromApp, idxToOs);

    int idxFromOS = fromOSApps->newGate("fromOSApps");
    fromOSApps->connectIn(scToApp, idxFromOS);

}

int VmMsgController::unlinkApplication(cModule* jobAppModule){

    int gateIdx = jobAppModule->gate("fromOS")->getPreviousGate()->getId();
    int position = toApps->searchGate(gateIdx);

    toOSApps->freeGate(position);
    toApps->freeGate(position);

    // Connections to SyscallManager
    fromApps->freeGate(position);
    fromOSApps->freeGate(position);

    return position;

}

void VmMsgController::setId(int userId, int vmId){
    uId = userId;
    pId = vmId;

}


void VmMsgController::insertCommId(int uId, int pId, int commId, int msgId){

    // Define ..
    bool found;
    commIdVector* comm;
    commIdVectorInternals* internals;

    // Initialize ..
    found = false;

    // Search at structure if there is an entry for the same VM
    for (int i = 0; (i < (int) commVector.size()) && (!found); i++) {

        // The VM exists
        if (((*(commVector.begin() + i))->uId == uId)
                && ((*(commVector.begin() + i))->pId == pId)) {

            // Create the new entry
            internals = new commIdVectorInternals();
            internals->msgId = msgId;
            internals->commId = commId;

            // Add the new entry to the structure
            (*(commVector.begin() + i))->internals.push_back(internals);

            found = true;      // break the loop
        }
    }

    // There is no entry for the vector..-
    if (!found) {
        // Create the general entry
        comm = new commIdVector();
        comm->uId = uId;
        comm->pId = pId;
        comm->internals.clear();

        // Create the concrete entry for the message
        internals = new commIdVectorInternals();

        internals->msgId = msgId;
        internals->commId = commId;
        comm->internals.push_back(internals);

        commVector.push_back(comm);
    }
}

void VmMsgController::updateCommId (Packet *pkt){
    // Define ..
    bool found;
    commIdVector* comm;
    commIdVectorInternals* internals;

    // Initialize ..
    found = false;
    pkt->trimFront();
    auto sm = pkt->removeAtFront<icancloud_App_NET_Message>();

    for (int i = 0; i < (int) commVector.size(); i++) {
        comm = (*(commVector.begin() + i));
        for (int j = 0; (j < (int) comm->internals.size()) && (!found); j++) {
            internals = (*(comm->internals.begin() + j));
        }
    }

    for (int i = 0; (i < (int) commVector.size()) && (!found); i++) {

        comm = (*(commVector.begin() + i));

        if ((sm->getUid() == comm->uId) && (sm->getPid() == comm->pId)) {

            for (int j = 0; (j < (int) comm->internals.size()) && (!found); j++) {

                internals = (*(comm->internals.begin() + j));

                if (internals->msgId == pkt->getId()) {
                    sm->setCommId(internals->commId);
                    comm->internals.erase(comm->internals.begin() + j);
                    found = true;
                }
            }

            if ((found) && ((int) comm->internals.size() == 0)) {
                commVector.erase(commVector.begin() + i);
            }
        }
    }
    pkt->insertAtFront(sm);
}

} // namespace icancloud
} // namespace inet
