#include "SimpleDisk.h"

namespace inet {

namespace icancloud {


Define_Module (SimpleDisk);


SimpleDisk::~SimpleDisk(){
    cancelAndDelete(latencyMessage);
}


void SimpleDisk::initialize(int stage){

    IStorageDevice::initialize(stage);
	if (stage == INITSTAGE_PHYSICAL_OBJECT_CACHE)
    {
        std::ostringstream osStream;

		// Set the moduleIdName
		osStream << "SimpleDisk." << getId();
		moduleIdName = osStream.str();
		// Init the super-class
		// Assign ID to module's gates
		inGate = gate ("in");
		outGate = gate ("out");

		// Get the blockSize
		readBandwidth = par ("readBandwidth");
		writeBandwidth = par ("writeBandwidth");

		// Latency message
		latencyMessage = new cMessage (SM_LATENCY_MESSAGE.c_str());
    }
}


void SimpleDisk::finish(){   

	// Finish the super-class
	icancloud_Base::finish();
}


cGate* SimpleDisk::getOutGate (cMessage *msg){

		// If msg arrive from scheduler
		if (msg->getArrivalGate()==inGate){
			if (outGate->getNextGate()->isConnected()){
				return (outGate);
			}
		}

	// If gate not found!
	return nullptr;
}


void SimpleDisk::processSelfMessage (cMessage *msg){

	//icancloud_BlockList_Message *sm_bl;


		// Latency message...
		if (msg == latencyMessage){
		    string Path = this->getFullPath();
		    if (pendingMessage == nullptr)
		        throw cRuntimeError("pendingMessage == NULL");
			// Cast!
		    const auto & sm_bl = pendingMessage->peekAtFront<icancloud_BlockList_Message>();
			if (sm_bl == nullptr)
			    throw cRuntimeError("Error header type");

			// Init pending message...


			// Change disk state to idle
			changeState (DISK_IDLE);

			// Send message back!
            auto pkt = pendingMessage;
            pendingMessage=nullptr;
			sendResponseMessage (pkt);
		}

		else
			showErrorMessage ("Unknown self message [%s]", msg->getName());
}


void SimpleDisk::processRequestMessage (Packet *pkt){

    int operation;


    const auto &sm = pkt->peekAtFront<icancloud_Message>();
    operation= sm->getOperation();
    if (operation == SM_CHANGE_DISK_STATE){
        changeDeviceState(sm->getChangingState().c_str());
        delete(pkt);
    }
    else {
        if (latencyMessage->isScheduled()) {
            throw cRuntimeError("Already pending");
        }
        // Link pending message

        pendingMessage = pkt;
        // Process the current IO request
        requestTime = service(pkt);
        changeState (DISK_ACTIVE);
        // Schedule a selft message to wait the request time...

        scheduleAt (simTime()+requestTime, latencyMessage);
        if (pendingMessage == nullptr)
            throw cRuntimeError("pendingMessage == NULL");
        string Path = this->getFullPath();
    }
}


void SimpleDisk::processResponseMessage (Packet *pkt){
	showErrorMessage ("There is no response messages in Disk Modules!!!");
}


simtime_t SimpleDisk::service(Packet *pkt){


    simtime_t totalDelay;           // Time to perform the IO operation
    simtime_t transferTime;
    pkt->trimFront();
    auto sm = pkt->removeAtFront<icancloud_Message>();

    size_blockList_t totalBlocks = 0;       // Total Number of blocks
    // Init
    //totalDelay = 0;

        // Cast to Standard message
     auto sm_bl = dynamicPtrCast<icancloud_BlockList_Message>(sm);
     // Get the number of blocks
     //totalBlocks =  sm_bl->getFile().getTotalSectors ();
     if (sm_bl == nullptr) throw cRuntimeError("Maybe the file that has been requested has not properly pwd setted..\n");

     if (sm_bl->getOperation() == 'r')
         transferTime = ((double)sm_bl->getSize()/(double)(readBandwidth*MB));
     else
         transferTime = ((double)sm_bl->getSize()/(double)(writeBandwidth*MB));

        // Calculate total time!
        //totalDelay = storageDeviceTime (totalBlocks, operation);

     if (DEBUG_Disk)
         showDebugMessage ("Processing request. Time:%f  - Requesting %lu bytes",
                                transferTime.dbl(),
                                totalBlocks);
     // Update message length
     sm_bl->updateLength();

     // Transfer data
     sm_bl->setIsResponse(true);
     pkt->insertAtFront(sm_bl);

    return transferTime;
}


simtime_t SimpleDisk::storageDeviceTime (size_blockList_t numBlocks, char operation){

	simtime_t transferTime;					// Transfer time
	unsigned long long int totalBytes;		// Total Number of bytes

		// Init...
		transferTime = 0.0;
		totalBytes = numBlocks*BYTES_PER_SECTOR;

		if (operation == 'r')
			transferTime = (simtime_t) ((simtime_t)totalBytes/(readBandwidth*MB));
		else
			transferTime = (simtime_t) ((simtime_t)totalBytes/(writeBandwidth*MB));

	return (transferTime);
}


void SimpleDisk::changeDeviceState (const string &state, unsigned componentIndex){

	if (state == MACHINE_STATE_IDLE) {

		nodeState = MACHINE_STATE_IDLE;
		changeState (DISK_IDLE);

	}
	else if (state == MACHINE_STATE_RUNNING) {

		nodeState = MACHINE_STATE_RUNNING;
		changeState (DISK_IDLE);

	}
	else if (state == MACHINE_STATE_OFF) {

		nodeState = MACHINE_STATE_OFF;
		changeState (DISK_OFF);
	}
}


void SimpleDisk::changeState (const string &energyState, unsigned componentIndex){

//	if (strcmp (nodeState.c_str(),MACHINE_STATE_OFF ) == 0) {
//		energyState = DISK_OFF;
//	}

//	if (strcmp (energyState.c_str(), DISK_OFF) == 0) nullptr;
//
//	else if (strcmp (energyState.c_str(), DISK_IDLE) == 0) nullptr;
//
//	else if (strcmp (energyState.c_str(), DISK_ACTIVE) == 0) nullptr;
//
// 	else nullptr;

	e_changeState (energyState);
}

} // namespace icancloud
} // namespace inet
