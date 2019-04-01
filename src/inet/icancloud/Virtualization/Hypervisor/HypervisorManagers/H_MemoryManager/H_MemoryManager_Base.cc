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

#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_MemoryManager/H_MemoryManager_Base.h"

namespace inet {

namespace icancloud {


H_MemoryManager_Base::~H_MemoryManager_Base() {
}

void H_MemoryManager_Base::initialize(int stage) {
    icancloud_Base::initialize (stage);

    if (stage == INITSTAGE_LOCAL) {
        std::ostringstream osStream;

        nodeGate = 0;

        blockSize_KB = par("blockSize_KB").intValue();
        memorySize_MB = par("memorySize_MB").intValue();

        // Init the gate IDs to/from CPU
        fromVMMemoryI = nullptr;
        fromVMMemoryO = nullptr;
        toVMMemoryI = nullptr;
        toVMMemoryO = nullptr;

        // Init the gates IDs to/from Vms
        fromNodeMemoryI = nullptr;
        toNodeMemoryI = nullptr;
        fromNodeMemoryO = nullptr;
        toNodeMemoryO = nullptr;

        // Init the gate IDs to/from CPU

        fromVMMemoryI = new cGateManager(this);
        fromVMMemoryO = new cGateManager(this);
        toVMMemoryI = new cGateManager(this);
        toVMMemoryO = new cGateManager(this);

        fromVMMemoryI->linkGate("fromVMMemoryI", nodeGate);
        fromVMMemoryO->linkGate("fromVMMemoryO", nodeGate);
        toVMMemoryI->linkGate("toVMMemoryI", nodeGate);
        toVMMemoryO->linkGate("toVMMemoryO", nodeGate);

        // Init the gates IDs to/from Vms
        fromNodeMemoryI = gate("fromNodeMemoryI");
        toNodeMemoryI = gate("toNodeMemoryI");

        fromNodeMemoryO = gate("fromNodeMemoryO");
        toNodeMemoryO = gate("toNodeMemoryO");

        vms.clear();

        memory_overhead_MB = par("memoryOverhead_MB");
        nodeOn = false;

    }
}

void H_MemoryManager_Base::finish(){
    vms.clear();
	icancloud_Base::finish();
}

void H_MemoryManager_Base::processSelfMessage (cMessage *msg){

    std::ostringstream msgLine;
    msgLine << "Unknown self message [" << msg->getName() << "]";
    throw cRuntimeError(msgLine.str().c_str());

    delete (msg);

}

cGate* H_MemoryManager_Base::getOutGate (cMessage *msg){

	// If msg arrive from VM
    if (msg->arrivedOn("fromVMMemoryI"))
        return (gate("toVMMemoryI", msg->getArrivalGate()->getIndex()));

    // If msg arrive from Node
    else if (msg->arrivedOn("fromNodeMemoryI"))
                return (gate("toNodeMemoryI"));

    // If msg arrive from VM
    else if (msg->arrivedOn("fromVMMemoryO"))
        return (gate("toVMMemoryO", msg->getArrivalGate()->getIndex()));

    // If msg arrive from Node
    else if (msg->arrivedOn("fromNodeMemoryO"))
        return (gate("toNodeMemoryO"));

    else
        // If gate not found!
        return nullptr;
}

void H_MemoryManager_Base::processRequestMessage (Packet *pkt){



    const auto  &sm = CHK(pkt->peekAtFront<icancloud_Message>());

	// Define ..
	/*	icancloud_App_MEM_Message *sm_mem;
		icancloud_Message *sm_mem_base;
		icancloud_Migration_Message *sm_migration;*/
	int operation;

	// Begin ..

		// Check the message operation ..
        operation = sm->getOperation();

	// The operation is a device state change
    if (operation == SM_CHANGE_DISK_STATE)

        sendRequestMessage(pkt, toVMMemoryO->getGate(pkt->getArrivalGate()->getIndex()));

    else if (operation == SM_CHANGE_MEMORY_STATE){

        // the node is turning on
        if (strcmp(sm->getChangingState().c_str(),"off") != 0) {

            if ((!nodeOn) && (memory_overhead_MB != 0.0)){
                nodeOn = true;

                // Create a new message!
                auto sm_mem = makeShared<icancloud_App_MEM_Message>();

                // Base parameters...
                sm_mem->setOperation (SM_MEM_ALLOCATE);
                sm_mem->setUid(0);
                sm_mem->setPid(this->getId());
                sm_mem->setMemSize(memory_overhead_MB * 1024);
                // Update message length
                sm_mem->updateLength ();
                auto pktMem = new Packet("icancloud_App_MEM_Message");
                pktMem->insertAtFront(sm_mem);

                // send the message allocating the overhead of the hypervisor
                sendRequestMessage(pktMem, toNodeMemoryI);
            }
        }

        // send the request message to change the memory state
        sendRequestMessage(pkt, toNodeMemoryI);

    }
    // The operation is a system operation
	else if (operation == ALLOCATE_MIGRATION_DATA)  {

        const auto &sm_migration = CHK(dynamicPtrCast<const icancloud_Migration_Message> (sm));

        // Analyze the contents of the message.
        auto sm_mem = migrationToMemoryContents(sm_migration);

        // Return the message allocation. The message copy about perform operations on memory had been created
        pkt->trimFront();
        auto sm_migrationAux = pkt->removeAtFront<icancloud_Migration_Message>();
        sm_migrationAux->setIsResponse(true);
        pkt->insertAtFront(sm_migrationAux);

        sendResponseMessage(pkt);

        auto pktMem = new Packet("icancloud_App_MEM_Message");
        pktMem->insertAtFront(sm_mem);

#ifdef COPYCONTROLINFO
        // I am not sure if it is necessary to copy the control info, the original code copy it, I cannot find sense
        if (pkt->getControlInfo()) {
            auto controlOld = check_and_cast<TcpCommand *>(pkt->getControlInfo());
            pktMem->setControlInfo (controlOld->dup());
        }
#endif

        // Perform the I/O Operation
        schedulingMemory(pktMem);
    }
    else if ((operation == GET_MIGRATION_DATA) || ((operation == GET_MIGRATION_CONNECTIONS))){

        const auto sm_migration = CHK(dynamicPtrCast<const icancloud_Migration_Message> (sm));
        //sm_migration = check_and_cast <icancloud_Migration_Message*> (msg);

        if (sm_migration->getMemorySizeKB() != 0){

            auto sm_mem = migrationToMemoryContents(sm_migration);
            sm_mem->setOperation(SM_MEM_RELEASE);
            pkt->trimFront();
            auto sm_migrationAux = pkt->removeAtFront<icancloud_Migration_Message>();
            sm_migrationAux->setMemorySizeKB(sm_mem->getMemSize() / KB);
            pkt->insertAtFront(sm_migrationAux);

//            sm_migration->setMemorySizeKB((cell->getTotalVMMemory()-cell->getRemainingVMMemory()) * blockSize_KB * 1024);
//            cell->freeAllMemory();

            //sm_mem_base = check_and_cast <icancloud_Message*> (sm_mem);

            auto pktMem = new Packet("icancloud_App_MEM_Message");
            pktMem->insertAtFront(sm_mem);

            //Send the message to the memory devices
            sendRequestMessage(pktMem, toNodeMemoryI);
        }

        // The message has to be returned to its owner
        pkt->trimFront();
        auto  smAux = CHK(pkt->removeAtFront<icancloud_Message>());
        smAux->setResult (RETURN_MESSAGE);
        smAux->setIsResponse(true);
        smAux->setResult(MEMORY_DATA);
        pkt->insertAtFront(smAux);
        // Send message back!
        sendResponseMessage (pkt);
        // The remote storage app check this value to set the connections in the block or not ..
        //    msg->setResult(MEMORY_DATA);
    }
    else if ((operation == SM_SET_IOR) || (operation == SM_CREATE_FILE) ||
            (operation == SM_DELETE_FILE) || (operation == SM_OPEN_FILE) ||
            (operation == SM_CLOSE_FILE) || (operation == SM_DELETE_USER_FS)
            ){

        if (sm->getRemoteOperation()){
            sendRequestMessage(pkt, toVMMemoryI->getGate(nodeGate));
        }
        else {
            int idx = getVMGateIdx(sm->getUid(), sm->getPid());

            if (idx == -1){
                sendRequestMessage(pkt, toVMMemoryO->getGate(nodeGate));

            }
            else{
                sendRequestMessage(pkt, toVMMemoryO->getGate(idx));
            }
        }
    }
    else
        schedulingMemory(pkt);

}

void H_MemoryManager_Base::processResponseMessage (Packet *pkt){

    auto sm = pkt->peekAtFront<icancloud_Message>();
    if (sm->getResult() == MEMORY_DATA){
	    delete(pkt);

	} else {
	    if ((sm->getUid() == 0) && (this->getId() == sm->getPid()))
	        delete (pkt);
	    else
	        sendResponseMessage(pkt);

	}
}


int H_MemoryManager_Base::setVM (cGate* oGateI, cGate* oGateO, cGate* iGateI, cGate* iGateO, int requestedMemory_KB, int uId, int pId){

    int idxToVMi;
    int idxToVMo;
    int idxFromVMi;
    int idxFromVMo;

    // Initialize control structure at node
        vmControl* control;
        control = new vmControl();
        control->gate = -1;
        control->uId = uId;
        control->pId = pId;

    // Connect to output gates
        idxToVMi = toVMMemoryI->newGate("toVMMemoryI");
        toVMMemoryI->connectOut(iGateI,idxToVMi);

        idxToVMo = toVMMemoryO->newGate("toVMMemoryO");
        toVMMemoryO->connectOut(iGateO,idxToVMo);

    // Connect to input gates
        idxFromVMo = fromVMMemoryO->newGate("fromVMMemoryO");
        fromVMMemoryO->connectIn(oGateO,idxFromVMo);

        idxFromVMi = fromVMMemoryI->newGate("fromVMMemoryI");
        fromVMMemoryI->connectIn(oGateI,idxFromVMi);

        control->gate = idxFromVMo;
        vms.push_back(control);

        return idxToVMo;

}

void H_MemoryManager_Base::freeVM (int uId, int pId){

    bool found = false;
    vmControl* control;

    for (int i = 0; (i < (int)vms.size()) && (!found); i++){

        control = (*(vms.begin() + i));

        if ((control->uId == uId) && (control->pId == pId)){

            toVMMemoryI ->freeGate (control->gate);
            toVMMemoryO ->freeGate (control->gate);

            fromVMMemoryI->freeGate (control->gate);
            fromVMMemoryO->freeGate (control->gate);

            vms.erase(vms.begin()+i);
            found = true;
        }
    }

    if (!found) throw cRuntimeError ("H_CPUManager_Base::freeVM--> vm id %i not exists at hypervisor\n", pId);
}


void H_MemoryManager_Base::sendMemoryMessage(Packet *pkt) {
    int gateIdx;
    //Get the gate and the index of the arrival msg

    if (pkt->arrivedOn("fromVMMemoryI")) {
        sendRequestMessage(pkt, toNodeMemoryI);

    }
    else if (pkt->arrivedOn("fromVMMemoryO")) {

        sendRequestMessage(pkt, toNodeMemoryO);

    }
    else if (pkt->arrivedOn("fromNodeMemoryI")) {
        const auto &sm = CHK(pkt->peekAtFront<icancloud_Message>());
        gateIdx = getVMGateIdx(sm->getUid(), sm->getPid());
        if (gateIdx == -1) {
            printf("H_MemoryManager_Base::sendMemoryMessage(toVMMemoryI[1]->uid - %i::::pid - %i\n",
                    sm->getUid(), sm->getPid());
            delete (pkt);
        }
        else
            sendRequestMessage(pkt, toVMMemoryI->getGate(gateIdx));
    }
    else if (pkt->arrivedOn("fromNodeMemoryO")) {
        const auto &sm = CHK(pkt->peekAtFront<icancloud_Message>());
        gateIdx = getVMGateIdx(sm->getUid(), sm->getPid());

        if (gateIdx == -1) {
            sendRequestMessage(pkt, toVMMemoryO->getGate(nodeGate));
        } else {
            sendRequestMessage(pkt, toVMMemoryO->getGate(gateIdx));
        }

    }
    else {
        throw cRuntimeError(
                "H_MemoryManager_Base::sendMemoryMessage->Error in the arrival of a message");
    }
}

Ptr<icancloud_App_MEM_Message>  H_MemoryManager_Base::migrationToMemoryContents(const Ptr<const icancloud_Migration_Message> &sm) {

    //icancloud_App_MEM_Message *newMessage;
    //TCPCommand *controlNew;
    //TCPCommand *controlOld;
    int i;

    // Create a new message!
    auto newMessage = makeShared<icancloud_App_MEM_Message>();

    // Base parameters...
    newMessage->setOperation(sm->getOperation());
    newMessage->setIsResponse(sm->getIsResponse());
    newMessage->setRemoteOperation(sm->getRemoteOperation());
    newMessage->setConnectionId(sm->getConnectionId());
    newMessage->setCommId(sm->getCommId());
    newMessage->setSourceId(sm->getSourceId());
    newMessage->setNextModuleIndex(sm->getNextModuleIndex());
    newMessage->setResult(sm->getResult());
    newMessage->setUid(sm->getUid());
    newMessage->setPid(sm->getPid());
    newMessage->setParentRequest(sm->getParentRequest());

    if (sm->getMemorySizeKB() > 0) {

        newMessage->setOperation(SM_MEM_ALLOCATE);
        newMessage->setChunkLength(B(sm->getMemorySizeKB()));
        newMessage->setMemSize(sm->getMemorySizeKB() / KB);

    } else if (sm->getMemorySizeKB() < 0) {

        newMessage->setOperation(SM_MEM_RELEASE);
        newMessage->setChunkLength(B(abs(sm->getMemorySizeKB())));
        newMessage->setMemSize(abs(sm->getMemorySizeKB()) / KB);

    } else {

        newMessage->setOperation(SM_MEM_SEARCH);

    }

    // Copy the control info, if exists!
    /*if (sm->getControlInfo() != nullptr) {
        controlOld = check_and_cast<TCPCommand *>(sm->getControlInfo());
        controlNew = new TCPCommand();
        controlNew = controlOld->dup();
        newMessage->setControlInfo(controlNew);
    }*/

    // Reserve memory to trace!
    newMessage->setTraceArraySize(sm->getTraceArraySize());

    // Copy trace!
    int size = sm->getTraceArraySize();
    for (i = 0; i < size; i++) {
        newMessage->addNodeTrace(sm->getHostName(i), sm->getNodeTrace(i));
    }

    return (newMessage);
}

} // namespace icancloud
} // namespace inet
