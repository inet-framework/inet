#include "RAMMemory_BlockModel.h"

namespace inet {

namespace icancloud {


Define_Module (RAMMemory_BlockModel);

using namespace inet;

RAMMemory_BlockModel::~RAMMemory_BlockModel(){
	
	cMessage *message;	

    	// Cancel the flush message
		cancelAndDelete (flushMessage);
		
		// Removes all messages placed on SMS
		SMS_memory->clear();
		
		// Removes the SMS object
		delete (SMS_memory);
		
		while (!memoryBlockList.empty()){
			delete (memoryBlockList.front());
			memoryBlockList.pop_front();
		}
		
		// Renoves all messages in memory list
		memoryBlockList.clear();
		
		// Removes all messages in flushQueue
		while (!flushQueue.isEmpty()){
			message = (cMessage *) flushQueue.pop();
			delete (message);			
		}
		
		flushQueue.clear();	

}


void RAMMemory_BlockModel::initialize(int stage) {

    HWEnergyInterface::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {

        std::ostringstream osStream;

        // Set the moduleIdName
        osStream << "MainMemory." << getId();
        moduleIdName = osStream.str();

        // Init the super-class

        waitingForOperation = false;

        // Module parameters
        readAheadBlocks = par("readAheadBlocks");
        size_KB = (par("size_MB").intValue()) * 1024;
        sizeCache_KB = par("sizeCache_KB");
        blockSize_KB = par("blockSize_KB");
        readLatencyTime_s = par("readLatencyTime_s");
        writeLatencyTime_s = par("writeLatencyTime_s");
        flushTime_s = par("flushTime_s");
        searchLatencyTime_s = par("searchLatencyTime_s");

        numDRAMChips = par("numDRAMChips");
        numModules = par("numModules");

        // Check sizes...
        if ((blockSize_KB <= 0) || (sizeCache_KB <= 0) || (size_KB <= 0)
                || (readAheadBlocks < 0))
            throw new cRuntimeError(
                    "MainMemory, wrong memory size or blocks sizes!!!");

        if ((size_KB % blockSize_KB) != 0)
            throw new cRuntimeError(
                    "MainMemory_CacheBlockLatencies, blockSize_KB must be multiple of size_KB!!!");

        if (sizeCache_KB > size_KB)
            throw new cRuntimeError(
                    "Cache size cannot be larger than the total memory size!!!");

        // Init variables
        totalBlocks = size_KB / blockSize_KB;
        totalCacheBlocks = sizeCache_KB / blockSize_KB;
        totalAppBlocks = totalBlocks - totalCacheBlocks;
        freeAppBlocks = totalAppBlocks;

        // Init the gate IDs to/from Input gates...
        toInputGates = gate("toInput");
        fromInputGates = gate("fromInput");

        // Init the gate IDs to/from Output
        toOutputGate = gate("toOutput");
        fromOutputGate = gate("fromOutput");

        // Create the latency message and flush message
        latencyMessage = new cMessage(SM_LATENCY_MESSAGE.c_str());
        flushMessage = new cMessage("flush-message");
        pendingMessage = nullptr;

        // Creates the SMS object
        SMS_memory = new SMS_MainMemory(size_KB * KB, readAheadBlocks,
                blockSize_KB * KB);

        // Flush Queue
        flushQueue.clear();

        nodeState = MACHINE_STATE_OFF;

        showStartedModule(
                "Init Memory: Size:%d bytes.  BlockSize:%d bytes.  Read-ahead:%d blocks - %d cache blocks - %d app. blocks",
                size_KB * KB, blockSize_KB * KB, readAheadBlocks,
                totalCacheBlocks, totalAppBlocks);
    }
}


void RAMMemory_BlockModel::finish(){

	// Finish the super-class
	icancloud_Base::finish();
}


cGate* RAMMemory_BlockModel::getOutGate (cMessage *msg){

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



void RAMMemory_BlockModel::processCurrentRequestMessage (){

    Packet *unqueuedMessage;

    // If exists enqueued requests
    if (!queue.isEmpty()) {

        // Pop!
        unqueuedMessage = (Packet*) queue.pop();

        // Dynamic cast!
        auto sm = unqueuedMessage->peekAtFront<icancloud_Message>();
        if (sm == nullptr)
            throw cRuntimeError("header is erroneous icancloud_Message");

        // Process
        processRequestMessage(unqueuedMessage);
    }
    else
        processSubRequests();
}


void RAMMemory_BlockModel::processSelfMessage (cMessage *msg){

    // Is a pending message?

    Ptr<icancloud_App_MEM_Message> sm_mem = nullptr;
    Ptr<icancloud_App_IO_Message> sm_io = nullptr;

    auto pkt = dynamic_cast<Packet *>(msg);

    if (pkt) {
        pkt->trim();
        auto sm = pkt->removeAtFront<icancloud_Message>();
        sm_mem = dynamicPtrCast <icancloud_App_MEM_Message> (sm);
        sm_io = dynamicPtrCast <icancloud_App_IO_Message> (sm);
        pkt->insertAtFront(sm);
    }



    if (!strcmp(msg->getName(), SM_LATENCY_MESSAGE.c_str())) {

        // There is a pending message...
        if (pendingMessage != nullptr) {

            if ((DEBUG_DETAILED_Main_Memory) && (DEBUG_Main_Memory)) {
                pendingMessage->trimFront();
                auto smAux = pendingMessage->removeAtFront<icancloud_Message>();
                showDebugMessage(
                        "End of memory processing! Pendign request:%s ",
                        smAux->contentsToString(DEBUG_MSG_Main_Memory).c_str());
                pendingMessage->insertAtFront(smAux);
            }

            cancelEvent(msg);

            sendRequest(pendingMessage);
        }

        // No pending message waiting to be sent...
        else {

            if ((DEBUG_DETAILED_Main_Memory) && (DEBUG_Main_Memory))
                showDebugMessage(
                        "End of memory processing! No pending request.");

            cancelEvent(msg);

            // Process next subRequest...
            processCurrentRequestMessage();
            //processSubRequests ();
        }
    }

    // Is a flush message?
    else if (!strcmp(msg->getName(), "flush-message")) {

        // There is no blocks on flush queue... stop the timer!
        if (!flushQueue.isEmpty())
            flushMemory();
    }

    else if (!strcmp(msg->getName(), SM_CHANGE_STATE_MESSAGE.c_str())) {
        cancelAndDelete(msg);
        changeState(MEMORY_STATE_IDLE);

    }
    else if (sm_io != nullptr) {
        cancelEvent(msg);
        changeState(MEMORY_STATE_IDLE);
        sendRequest (pkt);
    }
    else if (sm_mem != nullptr) {
        if (e_getActualState() == MEMORY_STATE_WRITE) {
            cancelEvent(msg);
            changeState(MEMORY_STATE_IDLE);
            sendResponseMessage (pkt);
        }
        else if (e_getActualState() == MEMORY_STATE_SEARCHING) {
            cancelEvent(msg);
            changeState(MEMORY_STATE_READ);
            // delete msg?
            processSubRequests();
        }
    }
    else
        showErrorMessage("Unknown self message [%s]", msg->getName());

}


void RAMMemory_BlockModel::processRequestMessage (Packet *pkt){

    pkt->trimFront();
    auto sm = pkt->removeAtFront<icancloud_Message>();

	auto sm_io = dynamicPtrCast<icancloud_App_IO_Message>(sm);
	auto sm_mem = dynamicPtrCast<icancloud_App_MEM_Message>(sm);
	unsigned int requiredBlocks;
	int operation = sm->getOperation();
	
	// Changing energy state operations came from OS
    if (operation == SM_CHANGE_MEMORY_STATE){

        // change the state of the memory
        changeDeviceState (sm->getChangingState().c_str());
        pkt->insertAtFront(sm);
        processCurrentRequestMessage();
        delete(pkt);


    }
    else if (operation == SM_CHANGE_DISK_STATE){

        // Cast!
         if (sm_io == nullptr)
             throw cRuntimeError("Header is not of the type icancloud_App_IO_Message");

        // Send request
        pkt->insertAtFront(sm_io);
        sendRequest (pkt);

    // Allocating memory for application space!
    }
    else if (sm->getOperation() == SM_MEM_ALLOCATE){

        changeState (MEMORY_STATE_WRITE);

		// Cast!
        if (sm_mem == nullptr)
            throw cRuntimeError("Header is not of the type icancloud_App_MEM_Message");

		
		// Memory account
		requiredBlocks = (unsigned int) ceil (sm_mem->getMemSize() / blockSize_KB);
				
		if (DEBUG_Main_Memory)
			showDebugMessage ("Memory Request. Free memory blocks: %d - Requested blocks: %d", freeAppBlocks, requiredBlocks);
		
		if (requiredBlocks <= freeAppBlocks)
			freeAppBlocks -= requiredBlocks;
		else{
			
			showDebugMessage ("Not enough memory!. Free memory blocks: %d - Requested blocks: %d", freeAppBlocks, requiredBlocks);			
			sm_mem->setResult (SM_NOT_ENOUGH_MEMORY);			
		}		
		
		// Response message
		sm_mem->setIsResponse(true);						

        // Time to perform the read operation
		pkt->insertAtFront(sm_mem);
        scheduleAt (writeLatencyTime_s+simTime(), pkt);

	}
	
	
	// Releasing memory for application space!
	else if (sm->getOperation() == SM_MEM_RELEASE){
				
	    changeState (MEMORY_STATE_WRITE);
		// Cast!
        if (sm_mem == nullptr)
            throw cRuntimeError("Header is not of the type icancloud_App_MEM_Message");
		
		// Memory account
		requiredBlocks = (unsigned int) ceil (sm_mem->getMemSize() / blockSize_KB);		
		
		if (DEBUG_Main_Memory)
			showDebugMessage ("Memory Request. Free memory blocks: %d - Released blocks: %d", freeAppBlocks, requiredBlocks);
		
		
		// Update number of free blocks
		freeAppBlocks += requiredBlocks;				
		
		// Response message
		sm_mem->setIsResponse(true);

        // Time to perform the write operation
		pkt->insertAtFront(sm_mem);
        scheduleAt (writeLatencyTime_s+simTime(), pkt);

	}
	
	
	// Disk cache space!
	else{

		// Cast!
        if (sm_io == nullptr)
            throw cRuntimeError("Header is not of the type icancloud_App_IO_Message");

        string debugMsg = sm_io->contentsToString(DEBUG_MSG_Main_Memory);
        pkt->insertAtFront(sm_io);

		// Read or write operation?
		if ((sm_io->getOperation() == SM_READ_FILE) ||
			(sm_io->getOperation() == SM_WRITE_FILE)){

			// Request cames from Service Redirector... Split it and process subRequests!
			if (!sm_io->getRemoteOperation()){

				// Split and process current request!
                SMS_memory->splitRequest(pkt);

				// Verbose mode? Show detailed request
				if (DEBUG_Main_Memory) {
					showDebugMessage ("Processing original request:%s from message:%s", 
									SMS_memory->requestToString(pkt, DEBUG_SMS_Main_Memory).c_str(),
									debugMsg.c_str());
				}



				showSMSContents(DEBUG_ALL_SMS_Main_Memory);

                changeState(MEMORY_STATE_SEARCHING);

                scheduleAt (searchLatencyTime_s+simTime(), pkt);

			}

			// Request cames from I/O Redirector... send to corresponding App (NFS)!
			else {
				sendRequest (pkt);
			}
		}

		// Control operation...
		else{
			sendRequest (pkt);
		}
	}
}

void RAMMemory_BlockModel::processResponseMessage(Packet *pkt) {

    // Response cames from remote node...
    if (pkt->arrivedOn("fromInput")) {
        sendResponseMessage (pkt);
    }
    // Response cames from I/O Redirector... (local)
    else {
        // Read or Write operation?
        pkt->trimFront();
        auto sm = pkt->removeAtFront<icancloud_Message>();

        auto sm_io = dynamicPtrCast<icancloud_App_IO_Message>(sm);
        auto sm_mem = dynamicPtrCast<icancloud_App_MEM_Message>(sm);

        if ((sm->getOperation() == SM_READ_FILE)
                || (sm->getOperation() == SM_WRITE_FILE)) {

            // Cast!
            if (sm_io == nullptr)
                throw cRuntimeError(
                        "Header is not of the type icancloud_App_IO_Message");

            // Update memory...
            icancloud_MemoryBlock* currentBlockCached;
            currentBlockCached = searchMemoryBlock(sm_io->getFileName(),
                    sm_io->getOffset());

            // If block in memory... set current block as NOT pending and re-insert on memory!
            if (currentBlockCached != nullptr) {
                currentBlockCached->setIsPending(false);
                reInsertBlock (currentBlockCached);
            }

            // Arrives subRequest!
            pkt->insertAtFront(sm_io);
            arrivesSubRequest(pkt);

            // Show memory contents?
            if ((DEBUG_SHOW_CONTENTS_Main_Memory) && (DEBUG_Main_Memory))
                showDebugMessage("Arrives block response. Memory contents: %s ",
                        memoryListToString().c_str());
        }

        // Control operation?
        else {
            pkt->insertAtFront(sm);
            sendResponseMessage(pkt);
        }
    }
}


void RAMMemory_BlockModel::sendRequest (Packet *pkt){

    const auto & sm = pkt->peekAtFront<icancloud_Message>();
	// Send to destination! I/O Redirector...
	if (!sm->getRemoteOperation())
		sendRequestMessage (pkt, toOutputGate);

	// Request cames from I/O Redirector... send to corresponding App!
	else{	
		sendRequestMessage (pkt, toInputGates);
	}
}


void RAMMemory_BlockModel::sendRequestWithoutCheck (Packet *pkt){

    const auto & sm = pkt->peekAtFront<icancloud_Message>();
	// If trace is empty, add current hostName, module and request number
	if (sm->isTraceEmpty()){
	    pkt->trimFront();
	    auto sm = pkt->removeAtFront<icancloud_Message>();
		sm->addNodeToTrace (getHostName());
		pkt->insertAtFront(sm);
		updateMessageTrace (pkt);
	}

	// Send to destination! Probably a file system...
	if (!sm->getRemoteOperation()){
		send (pkt, toOutputGate);
	}

	// Request cames from IOR... send to corresponding App!
	else{	
		send (pkt, toInputGates);
	}
}


void RAMMemory_BlockModel::processSubRequests() {

    bool is_memoryFull;							// Is the memory full?
    bool all_blocksPending;					// Are all cached blocks pending?
    icancloud_MemoryBlock* currentBlockCached;	// Is current block in memory?
    string isCached;						// Is block cached? in string format
    bool isMemorySaturated;						// Is cache saturated?
    bool currentSubReqIsPending;

    // Currently processing memory?
    if (latencyMessage->isScheduled()) {

        if (DEBUG_Main_Memory)
            showDebugMessage(
                    "Currently processing memory... current subRequest must wait!");

        return;
    }

    pendingMessage = nullptr;

    // Get the first subrequest!
    auto pkt = SMS_memory->getFirstSubRequest();
    pkt->trimFront();
    auto sm = pkt->removeAtFront<icancloud_Message>();
    const auto &sm_io = dynamicPtrCast<icancloud_App_IO_Message>(sm);

    pkt->insertAtFront(sm);

    // There is, at least, one subRequest...
    if (sm_io != nullptr) {

        // Check if current block is in memory!
        currentBlockCached = searchMemoryBlock(sm_io->getFileName(), sm_io->getOffset());

        // Is the block cached? parse to string...
        if (currentBlockCached == nullptr)
            isCached = "false";
        else
            isCached = "true";

        // Calculates is memory is full!
        is_memoryFull = isMemoryFull();

        // All blocks are pending?
        all_blocksPending = allBlocksPending();

        // Conditions to saturate memory...
        isMemorySaturated = ((is_memoryFull) && (all_blocksPending) && (currentBlockCached == nullptr));

        if (DEBUG_Main_Memory)
            showDebugMessage(
                    "Processing subRequest: Op:%s File:%s Offset:%u Size:%u",
                    sm_io->operationToString().c_str(), sm_io->getFileName(),
                    sm_io->getOffset(), sm_io->getSize());

        if (DEBUG_Main_Memory)
            showDebugMessage(
                    "Status: blockCached?:%s memoryFull?:%s allPending?:%s -> saturated?:%s",
                    isCached.c_str(), boolToString(is_memoryFull).c_str(),
                    boolToString(all_blocksPending).c_str(),
                    boolToString(isMemorySaturated).c_str());

        // Memory is saturated!!! Send directly the request!!!
        if (isMemorySaturated) {

            if ((DEBUG_DETAILED_Main_Memory) && (DEBUG_Main_Memory))
                showDebugMessage(
                        "Memory is saturated!!! Sending subRequest directly to destination...");

            // Pop subrequest!
            auto pktAux = SMS_memory->popSubRequest();

            // Send..
            sendRequest(pktAux);

            changeState(MEMORY_STATE_IDLE);
        }

        // Memory not saturated... using memory!!!
        else {

            auto pkt = SMS_memory->popSubRequest();
            pkt->trimFront();
            auto sm = pkt->removeAtFront<icancloud_Message>();
            auto sm_io = dynamicPtrCast<icancloud_App_IO_Message>(sm);

            // Is a READ operation?
            if (sm_io->getOperation() == SM_READ_FILE) {

                // Current block is cached!
                if (currentBlockCached != nullptr) {

                    // If not pending, re-insert the block!
                    if (!currentBlockCached->getIsPending())
                        reInsertBlock(currentBlockCached);

                    if ((DEBUG_DETAILED_Main_Memory) && (DEBUG_Main_Memory))
                        showDebugMessage(
                                "Processing memory... block cached! (latencyTime_s). Request:%s",
                                sm_io->contentsToString(DEBUG_MSG_Main_Memory).c_str());

                    // Latency time!
                    scheduleAt(readLatencyTime_s + simTime(), latencyMessage);

                    // Send back as solved request!
                    sm_io->setIsResponse(true);
                    pkt->insertAtFront(sm_io);
                    arrivesSubRequest(pkt);
                }

                // Current block is not cached!
                else {

                    // If memory full, remove a block!
                    if (is_memoryFull)
                        memoryBlockList.pop_back();

                    // Insert current block in memory!
                    insertBlock(sm_io->getFileName(), sm_io->getOffset(), sm_io->getSize());




                    if ((DEBUG_DETAILED_Main_Memory) && (DEBUG_Main_Memory))
                        showDebugMessage(
                                "Processing memory... block NOT cached! (latencyTime_s). Request:%s",
                                sm_io->contentsToString(DEBUG_MSG_Main_Memory).c_str());

                    // Send request...
                    pkt->insertAtFront(sm_io);
                    pendingMessage = pkt;
                    scheduleAt(readLatencyTime_s + simTime(), latencyMessage);

                }
            }

            // Is a WRITE operation?
            else {

                // Current block is cached!
                if (currentBlockCached != nullptr) {

                    // Current block is NOT pending!
                    if (!currentBlockCached->getIsPending()) {

                        if ((DEBUG_DETAILED_Main_Memory) && (DEBUG_Main_Memory))
                            showDebugMessage(
                                    "Processing memory... block cached! (latencyTime_s). Request:%s",
                                    sm_io->contentsToString(
                                            DEBUG_MSG_Main_Memory).c_str());

                        // Mark current block as pending!
                        currentBlockCached->setIsPending(true);

                        // Re insert current block...
                        reInsertBlock(currentBlockCached);

                        // enQueue on Flush Queue!
                        pkt->insertAtFront(sm_io);
                        insertRequestOnFlushQueue(pkt);

                        currentSubReqIsPending = false;
                    }

                    // Current block is pending...
                    else {
                        pkt->insertAtFront(sm_io);
                        currentSubReqIsPending = true;
                    }
                }

                // not in memory!
                else {

                    if ((DEBUG_DETAILED_Main_Memory) && (DEBUG_Main_Memory))
                        showDebugMessage(
                                "Processing memory... block NOT cached:%s",
                                sm_io->contentsToString(DEBUG_MSG_Main_Memory).c_str());

                    currentSubReqIsPending = false;

                    // If memory full, remove a block!
                    if (is_memoryFull)
                        memoryBlockList.pop_back();

                    // Insert on memory
                    insertBlock(sm_io->getFileName(), sm_io->getOffset(),
                            sm_io->getSize());

                    // enQueue on Flush Queue!
                    pkt->insertAtFront(sm_io);
                    insertRequestOnFlushQueue(pkt);
                }

                // Copy request to send response to SMS!
                auto pkt2 = pkt->dup();
                pkt2->trimFront();
                auto sm_io_copy = pkt2->removeAtFront<icancloud_App_IO_Message>();
                sm_io_copy->setIsResponse(true);
#ifdef COPYCONTROLINFO
                // I am not sure if it is necessary to copy the control info, the original code copy it, I cannot find sense
                if (pkt->getControlInfo()) {
                    auto controlOld = check_and_cast<TcpCommand *>(pkt->getControlInfo());
                    pkt2->setControlInfo (controlOld->dup());
                }
#endif
                scheduleAt(writeLatencyTime_s + simTime(), latencyMessage);
                pkt2->insertAtFront(sm_io_copy);
                arrivesSubRequest(pkt2);

                if (currentSubReqIsPending)
                    delete (pkt);

                // Flush!
                if (flushTime_s == 0)
                    flushMemory();

            }

            // Show memory contents?
            if ((DEBUG_SHOW_CONTENTS_Main_Memory) && (DEBUG_Main_Memory))
                showDebugMessage(" %s ", memoryListToString().c_str());
        }
    }
}


void RAMMemory_BlockModel::arrivesSubRequest (Packet *pkt){


    auto subRequest = pkt->peekAtFront<icancloud_App_IO_Message>();

	// Parent request
	auto pktParent = subRequest->getParentRequest();

	// Search for the request on request vector!
	bool isRequestHere = (SMS_memory->searchRequest (pktParent) != NOT_FOUND);
	// If request is not here... delete current subRequest!
	if (!isRequestHere){
	    if ((DEBUG_DETAILED_Main_Memory) && (DEBUG_Main_Memory)) {
	        pkt->trimFront();
	        auto subRequest = pkt->removeAtFront<icancloud_App_IO_Message>();
	        showDebugMessage ("Arrived subRequest has no parent.. deleting! %s", subRequest->contentsToString(DEBUG_MSG_Main_Memory).c_str());
	        pkt->insertAtFront(subRequest);
	    }
	    delete (pkt);
	}
	else{
	    // Casting!
	    // SubRequest arrives...
	    SMS_memory->arrivesSubRequest (pkt, pktParent);
	    // Check for errors...
	    if (subRequest->getResult() != icancloud_OK){
	        auto parentRequest = pktParent->removeAtFront<icancloud_App_IO_Message>();
	        parentRequest->setResult (subRequest->getResult());
	        pktParent->insertAtFront(parentRequest);
	    }


			// Verbose mode? Show detailed request
			if ((DEBUG_DETAILED_Main_Memory) && (DEBUG_Main_Memory)) {
			    pkt->trimFront();
			    auto subRequest = pkt->removeAtFront<icancloud_App_IO_Message>();
				showDebugMessage ("Processing request response:%s from message:%s", 
									SMS_memory->requestToString(pktParent, DEBUG_SMS_Main_Memory).c_str(),
									subRequest->contentsToString(DEBUG_MSG_Main_Memory).c_str());
				pkt->insertAtFront(subRequest);
			}
			
			
			// If all request have arrived...
			if ((SMS_memory->arrivesAllSubRequest(pktParent)) ||
				(SMS_memory->arrivesRequiredBlocks(pktParent, readAheadBlocks))){
							
				if ((DEBUG_DETAILED_Main_Memory) && (DEBUG_Main_Memory))
					showDebugMessage ("Arrives all subRequest!");

				// Removes the request object!
				SMS_memory->removeRequest (pktParent);

				// Show complete SMS
				showSMSContents (DEBUG_SHOW_CONTENTS_Main_Memory);
				pktParent->trimFront();
				auto parentRequest = pktParent->removeAtFront<icancloud_App_IO_Message>();
				// Now is a Response Message
				parentRequest->setIsResponse (true);
				// Update the mesage length!
				parentRequest->updateLength();
				pktParent->insertAtFront(parentRequest);

				// Send response
				sendResponseMessage (pktParent);
			}
		}
}


void RAMMemory_BlockModel::flushMemory (){

 Packet *unqueuedMessage;

	if (simTime() >= 2999)
		endSimulation();

	// If there is elements inside the queue
	while (flushQueue.getLength()>0){
		// pop next element...
		unqueuedMessage = (Packet *)flushQueue.pop();
		auto sm_io = unqueuedMessage->peekAtFront<icancloud_App_IO_Message>();
		if (sm_io == nullptr)
		    throw cRuntimeError("Header of incorrect type");
		sendRequestWithoutCheck (unqueuedMessage);
	}
}


void RAMMemory_BlockModel::insertRequestOnFlushQueue (Packet *pktSubRequest){

	// Insert a block on flush queue
    auto subRequest = pktSubRequest->peekAtFront<icancloud_App_IO_Message>();
    if (subRequest == nullptr)
        throw cRuntimeError("Header of incorrect type");
    // check type

	flushQueue.insert (pktSubRequest);

	// If memory Full!! Must force the flush...
	if (isMemoryFull()){

		// If there was activated a previous timer, then cancel it!
		if (flushMessage->isScheduled())
			cancelEvent (flushMessage);

		// Forced flush!
		flushMemory();
	}

	// If timer is not active... activate it!
	else if (!flushMessage->isScheduled()){
		scheduleAt (flushTime_s+simTime(), flushMessage);
	}
}


bool RAMMemory_BlockModel::isMemoryFull (){

	if (memoryBlockList.size() >= totalCacheBlocks)
		return true;
	else
		return false;
}


bool RAMMemory_BlockModel::allBlocksPending() {

    bool allPending;

    // Init
    allPending = true;

    // Is empty?
    if (memoryBlockList.empty())
        allPending = false;
    // Memory not empty!
    else {

        // Walk through the list searching the requested block!
        for (auto listIterator = memoryBlockList.begin(); (listIterator != memoryBlockList.end() && (allPending)); ++listIterator) {
            if (!(*listIterator)->getIsPending())
                allPending = false;
        }
    }
    return (allPending);
}


void RAMMemory_BlockModel::insertBlock (string fileName, unsigned int offset, unsigned int size){

	icancloud_MemoryBlock *block;
	int blockOffset;
	int blockNumber;


		// Bigger than memory block size...
		if (size > (blockSize_KB*KB))
			throw new cRuntimeError("[searchMemoryBlock] memory Block size too big!!!");

		// Block number and Block offset
		blockNumber = offset/(blockSize_KB*KB);
		blockOffset = blockNumber*(blockSize_KB*KB);

		if ((offset+size)>((blockNumber+1)*(blockSize_KB*KB))){
			throw new cRuntimeError("[searchMemoryBlock] Wrong memory Block size (out of bounds)!!!");
		}

		block = new icancloud_MemoryBlock();

		// Set corresponding values
		block->setFileName (fileName);
		block->setOffset (blockOffset);
		block->setBlockSize (blockSize_KB*KB);
		block->setIsPending (true);

		// Insert block on memory list!
		memoryBlockList.push_front (block);
}


void RAMMemory_BlockModel::reInsertBlock (icancloud_MemoryBlock *block){

	list <icancloud_MemoryBlock*>::iterator listIterator;
	icancloud_MemoryBlock* memoryBlock;
	bool found;


		// Init
		memoryBlock = nullptr;
		found = false;

		// Walk through the list searching the requested block!
		for (listIterator=memoryBlockList.begin();
			(listIterator!=memoryBlockList.end() && (!found));
			++listIterator){

			// Found?
			if ((block->getFileName() == (*listIterator)->getFileName()) &&
				(block->getOffset() == (*listIterator)->getOffset() )){
				found = true;
				memoryBlock = *listIterator;
				memoryBlockList.erase (listIterator);
				memoryBlockList.push_front (memoryBlock);
			}
		}
}


icancloud_MemoryBlock* RAMMemory_BlockModel::searchMemoryBlock (const char* fileName, unsigned int offset){

	list <icancloud_MemoryBlock*>::iterator listIterator;
	icancloud_MemoryBlock* memoryBlock;
	bool found;
	int currentBlock, requestedBlock;


		// Init
		memoryBlock = nullptr;
		found = false;
		requestedBlock = offset/(blockSize_KB*KB);

		// Walk through the list searching the requested block!
		for (listIterator=memoryBlockList.begin(); ((listIterator!=memoryBlockList.end()) && (!found)); listIterator++){

			currentBlock = 	((*listIterator)->getOffset())/(blockSize_KB*KB);

			if ((!strcmp (fileName, (*listIterator)->getFileName().c_str()) ) &&
				(requestedBlock==currentBlock)){
				found = true;
				memoryBlock = *listIterator;
			}
		}


	return (memoryBlock);
}


string RAMMemory_BlockModel::memoryListToString (){

	list <icancloud_MemoryBlock*>::iterator listIterator;
	std::ostringstream info;
	int currentBlock;


		// Init
		currentBlock = 0;
		info << "Memory list..." << endl;

			// Walk through the list searching the requested block!
			for (listIterator=memoryBlockList.begin();
				listIterator!=memoryBlockList.end();
				listIterator++){

					info << "Block[" << currentBlock << "]: " << (*listIterator)->memoryBlockToString() << endl;
					currentBlock++;
			}

	return info.str();
}


void RAMMemory_BlockModel::showSMSContents (bool showContents){

	int i;
	int numRequests;
	
		if (showContents){

			// Get the number of requests
			numRequests = SMS_memory->getNumberOfRequests();
			
			showDebugMessage ("Showing the complete SMS vector... (%d requests)", numRequests);
	
			// Get all requests
			for (i=0; i<numRequests; i++)			
				showDebugMessage (" Request[%d]:%s ", i, SMS_memory->requestToStringByIndex(i).c_str());
		}
}

void RAMMemory_BlockModel::changeDeviceState (const string & state,unsigned componentIndex){

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


void RAMMemory_BlockModel::changeState (const string & energyState,unsigned componentIndex ){

//  if (strcmp (nodeState.c_str(),MACHINE_STATE_OFF ) == 0) {
//      energyState = MEMORY_STATE_OFF;
//  }

//  if (strcmp (energyState.c_str(), MEMORY_STATE_READ) == 0) nullptr;
//
//  else if (strcmp (energyState.c_str(), MEMORY_STATE_WRITE) == 0) nullptr;
//
//  else if (strcmp (energyState.c_str(), MEMORY_STATE_IDLE) == 0) nullptr;
//
//  else if (strcmp (energyState.c_str(), MEMORY_STATE_OFF) == 0) nullptr;
//
//  else if (strcmp (energyState.c_str(), MEMORY_STATE_SEARCHING) == 0) nullptr;
//
//  else nullptr;


    e_changeState (energyState);

}


} // namespace icancloud
} // namespace inet
