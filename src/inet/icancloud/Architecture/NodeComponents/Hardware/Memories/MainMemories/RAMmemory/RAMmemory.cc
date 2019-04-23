#include "inet/icancloud/Architecture/NodeComponents/Hardware/Memories/MainMemories/RAMmemory/RAMmemory.h"

namespace inet {

namespace icancloud {


Define_Module (RAMmemory);


RAMmemory::~RAMmemory(){
    delete(operationMessage);
}


void RAMmemory::initialize(int stage) {

    HWEnergyInterface::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        std::ostringstream osStream;

        waitingForOperation = false;

        // Set the moduleIdName
        osStream << "BasicMainMemory." << getId();
        moduleIdName = osStream.str();

        // Module parameters
        size_MB = par("size_MB");
        blockSize_KB = par("blockSize_KB");
        readLatencyTime_s = par("readLatencyTime_s");
        writeLatencyTime_s = par("writeLatencyTime_s");
        searchLatencyTime_s = par("searchLatencyTime_s");
        numDRAMChips = par("numDRAMChips");
        numModules = par("numModules");

        // Check sizes...
        if ((blockSize_KB <= 0) || (size_MB <= 0))
            throw cRuntimeError("BasicMainMemory, wrong memory size!!!");

        // Init variables
        totalBlocks = (size_MB * 1000) / blockSize_KB;
        totalAppBlocks = 0;
        freeAppBlocks = totalBlocks;

        // Init the gate IDs to/from Input gates...
        toInputGates = gate("toInput");
        fromInputGates = gate("fromInput");

        // Init the gate IDs to/from Output
        toOutputGate = gate("toOutput");
        fromOutputGate = gate("fromOutput");

        // Create the latency message and flush message
        operationMessage = new cMessage(SM_CHANGE_STATE_MESSAGE.c_str());

        pendingMessage = nullptr;

        nodeState = MACHINE_STATE_OFF;

        showStartedModule(
                "Init Memory: Size:%d bytes.  BlockSize:%d bytes. %d app. blocks",
                size_MB * 1000 * 1000, blockSize_KB * KB, totalAppBlocks);
    }

}


void RAMmemory::finish(){

	// Finish the super-class
	icancloud_Base::finish();
}


cGate* RAMmemory::getOutGate (cMessage *msg){

		// If msg arrive from Output
		if (msg->getArrivalGate()==fromOutputGate){
			if (toOutputGate->getNextGate()->isConnected()){
				return (toOutputGate);
			}
		}

		// If msg arrive from Inputs
		else if (msg->arrivedOn("fromInput")){
					return (toInputGates);
		}

	// If gate not found!
	return nullptr;
}


void RAMmemory::processCurrentRequestMessage (){

	Packet *unqueuedMessage;

	// If exists enqueued requests
	if (!queue.isEmpty()){
	    // Pop!
	    unqueuedMessage = (Packet *) queue.pop();
	    // Dynamic cast!
	    const auto &sm = unqueuedMessage->peekAtFront<icancloud_Message>();
	    if (sm == nullptr)
	        throw cRuntimeError("Header type error");
	    // Process
	    processRequestMessage (unqueuedMessage);
	}
}


void RAMmemory::processSelfMessage (cMessage *msg){


    // Is a pending message?
    if (!strcmp(msg->getName(), SM_LATENCY_MESSAGE.c_str())) {

        // There is a pending message...
        if (pendingMessage != nullptr) {

            if ((DEBUG_DETAILED_Basic_Main_Memory) && (DEBUG_Basic_Main_Memory)) {
                const auto & sm  = pendingMessage->peekAtFront<icancloud_Message>();
                showDebugMessage("End of memory processing! Pending request:%s ",
                        sm->contentsToString(DEBUG_MSG_Basic_Main_Memory).c_str());
            }

            // Send current request
            sendRequest(pendingMessage);
        }

        // No pending message waiting to be sent...
        else {

            if ((DEBUG_DETAILED_Basic_Main_Memory) && (DEBUG_Basic_Main_Memory))
                showDebugMessage("End of memory processing! No pending request.");

            // Process next request
            processCurrentRequestMessage();
        }
        cancelEvent(msg);
        return;
    }


    if (!strcmp(msg->getName(), SM_CHANGE_STATE_MESSAGE.c_str())) {
        cancelAndDelete(msg);
        changeState(MEMORY_STATE_IDLE);
        return;

    }

    bool isIo = false;
    bool isMem = false;
    Packet *pkt = dynamic_cast<Packet *>(msg);
    if (pkt) {
        pkt->trimFront();
        auto sm = pkt->removeAtFront<icancloud_Message>();
        auto sm_io = dynamicPtrCast<icancloud_App_IO_Message>(sm);
        auto sm_mem = dynamicPtrCast<icancloud_App_MEM_Message>(sm);
        pkt->insertAtFront(sm);
        if (sm_io)
            isIo = true;
        if (sm_mem)
            isMem = true;
    }

    if (isIo) {
        cancelEvent(msg);
        changeState(MEMORY_STATE_IDLE);
        sendRequest(pkt);
    }
    else if (isMem) {
        cancelEvent(msg);
        changeState(MEMORY_STATE_IDLE);
        sendResponseMessage(pkt);
    }
    else
        showErrorMessage("Unknown self message [%s]", msg->getName());

}


void RAMmemory::processRequestMessage (Packet *pkt){

	unsigned int requiredBlocks;
	int operation;

	const auto &sm = pkt->peekAtFront<icancloud_Message>();
	operation = sm->getOperation();

	// Changing memory state from the node state!
	if (operation == SM_CHANGE_MEMORY_STATE){

		// change the state of the memory
		changeDeviceState (sm->getChangingState().c_str());
		processCurrentRequestMessage();
		delete(pkt);

	}
	else if (operation == SM_CHANGE_DISK_STATE){

		// Cast!
	    auto sm_io = pkt->peekAtFront<icancloud_App_IO_Message>();
	    if (sm_io == nullptr)
	        throw cRuntimeError("Header error");

		// Send request
		sendRequest (pkt);

    // Allocating memory for application space!

	}
	else if (operation == SM_MEM_ALLOCATE){

	    changeState (MEMORY_STATE_WRITE);
		// Cast!
	    pkt->trimFront();
		auto sm_mem = pkt->removeAtFront<icancloud_App_MEM_Message>();
        if (sm_mem == nullptr)
            throw cRuntimeError("Header error");
		
		// Memory account
		requiredBlocks = (unsigned int) ceil (sm_mem->getMemSize() / blockSize_KB);
				
		if (DEBUG_Basic_Main_Memory)
			showDebugMessage ("Memory Request. Free memory blocks: %d - Requested blocks: %d", freeAppBlocks, requiredBlocks);
		
		if (requiredBlocks <= freeAppBlocks)
			freeAppBlocks -= requiredBlocks;
		else{
			showDebugMessage ("Not enough memory!. Free memory blocks: %d - Requested blocks: %d", freeAppBlocks, requiredBlocks);
			sm_mem->setResult (SM_NOT_ENOUGH_MEMORY);			
		}		
		
		// Response message
		sm_mem->setIsResponse(true);
        pkt->insertAtFront(sm_mem);

		// Time to perform the read operation
		scheduleAt (writeLatencyTime_s+simTime(), pkt);

	}
	
	// Releasing memory for application space!
	else if (operation == SM_MEM_RELEASE){

	    changeState (MEMORY_STATE_WRITE);
		// Cast!
	    pkt->trimFront();
	    auto sm_mem = pkt->removeAtFront<icancloud_App_MEM_Message>();
		if (sm_mem == nullptr)
		    throw cRuntimeError("Header error");
		
		// Memory account
		requiredBlocks = (unsigned int) ceil (sm_mem->getMemSize() / blockSize_KB);		
		
		if (DEBUG_Basic_Main_Memory)
			showDebugMessage ("Memory Request. Free memory blocks: %d - Released blocks: %d", freeAppBlocks, requiredBlocks);
		
		
		// Update number of free blocks
		freeAppBlocks += requiredBlocks;
		if (freeAppBlocks > totalBlocks){
		    freeAppBlocks = totalBlocks;
		}
		
		// Response message
		sm_mem->setIsResponse(true);
		pkt->insertAtFront(sm_mem);

		// Time to perform the write operation
		scheduleAt (writeLatencyTime_s+simTime(), pkt);

	}
	
	
	// Disk cache space!
	else{

		// Cast!
		 const auto &sm_io = pkt->peekAtFront<icancloud_App_IO_Message>();
		 if (sm_io == nullptr)
		     throw cRuntimeError("Header error");


		// Read or write operation?
		if ((operation == SM_READ_FILE) ||
			(operation == SM_WRITE_FILE)){


			// Request came from Service Redirector... Split it and process subRequests!
			if (!sm_io->getRemoteOperation()){

				// Verbose mode? Show detailed request
				if (DEBUG_Basic_Main_Memory)
					showDebugMessage ("Processing request:%s",
									  sm_io->contentsToString(DEBUG_MSG_Basic_Main_Memory).c_str());
				// Search in the banks of the memory and request to the io server
				changeState(MEMORY_STATE_SEARCHING);
				scheduleAt (searchLatencyTime_s+simTime(), pkt);

			}

			// Request cames from I/O Redirector... send to NFS!
			else
				sendRequest (pkt);
		}

		// Control operation...
		else{
			sendRequest (pkt);
		}
	}
}


void RAMmemory::processResponseMessage (Packet *pkt){

	// Verbose mode? Show detailed request
	if (DEBUG_Basic_Main_Memory){
		auto sm_io = pkt->peekAtFront<icancloud_App_IO_Message>();
		showDebugMessage ("Sending response:%s",
					       sm_io->contentsToString(DEBUG_MSG_Basic_Main_Memory).c_str());
	}
	changeState (MEMORY_STATE_IDLE);
	sendResponseMessage (pkt);
}


void RAMmemory::sendRequest (Packet *pkt){

    const auto &sm = pkt->peekAtFront<icancloud_Message>();
	// Send to destination! I/O Redirector...
	if (!sm->getRemoteOperation())
		sendRequestMessage (pkt, toOutputGate);

	// Request cames from I/O Redirector... send to corresponding App!
	else{	

		send (pkt, toInputGates);
	}
}

void RAMmemory::changeDeviceState (const string & state,unsigned componentIndex){

	if (state == MACHINE_STATE_IDLE) {

		nodeState = MACHINE_STATE_IDLE;
		changeState (MEMORY_STATE_IDLE);

	}
	else if (state == MACHINE_STATE_RUNNING) {

		nodeState = MACHINE_STATE_RUNNING;
		changeState (MEMORY_STATE_IDLE);

	}
	else if (state == MACHINE_STATE_OFF) {

		nodeState = MACHINE_STATE_OFF;
        totalAppBlocks = 0;
        freeAppBlocks = totalBlocks;
		changeState (MEMORY_STATE_OFF);
	}
}


void RAMmemory::changeState (const string &  energyState,unsigned componentIndex ){

//	if (strcmp (nodeState.c_str(),MACHINE_STATE_OFF ) == 0) {
//		energyState = MEMORY_STATE_OFF;
//	}

//	if (strcmp (energyState.c_str(), MEMORY_STATE_READ) == 0) nullptr;
//
//	else if (strcmp (energyState.c_str(), MEMORY_STATE_WRITE) == 0) nullptr;
//
//	else if (strcmp (energyState.c_str(), MEMORY_STATE_IDLE) == 0) nullptr;
//
//	else if (strcmp (energyState.c_str(), MEMORY_STATE_OFF) == 0) nullptr;
//
//	else if (strcmp (energyState.c_str(), MEMORY_STATE_SEARCHING) == 0) nullptr;
//
// 	else nullptr;


	e_changeState (energyState);

}



} // namespace icancloud
} // namespace inet
