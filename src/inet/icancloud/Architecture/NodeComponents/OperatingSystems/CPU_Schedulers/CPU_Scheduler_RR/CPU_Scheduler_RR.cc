#include "CPU_Scheduler_RR.h"

namespace inet {

namespace icancloud {


Define_Module (CPU_Scheduler_RR);



CPU_Scheduler_RR::~CPU_Scheduler_RR(){
		
	requestsQueue.clear();			
}


void CPU_Scheduler_RR::initialize(int stage) {
    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        std::ostringstream osStream;
        unsigned int i;

        // Set the moduleIdName
        osStream << "CPU_Scheduler_RR." << getId();
        moduleIdName = osStream.str();

        // Init the super-class

        // Get module parameters
        numCPUs = par("numCPUs");
        quantum = par("quantum");

        // State of CPUs
        isCPU_Idle = new bool[numCPUs];

        // Init state to idle!
        for (i = 0; i < numCPUs; i++)
            isCPU_Idle[i] = true;

        // Init requests queue
        requestsQueue.clear();

        // Init the gate IDs to/from Scheduler
        fromOsGate = gate("fromOsGate");
        toOsGate = gate("toOsGate");

        // Init the gates IDs to/from BlockServers
        toCPUGate = new cGate*[numCPUs];
        fromCPUGate = new cGate*[numCPUs];

        for (i = 0; i < numCPUs; i++) {
            toCPUGate[i] = gate("toCPU", i);
            fromCPUGate[i] = gate("fromCPU", i);
        }
    }
}


void CPU_Scheduler_RR::finish(){

	// Finish the super-class
	icancloud_Base::finish();
}


cGate* CPU_Scheduler_RR::getOutGate (cMessage *msg){

	unsigned int i;

		// If msg arrive from Output
		if (msg->getArrivalGate()==fromOsGate){
			if (gate("toOsGate")->getNextGate()->isConnected()){
				return (toOsGate);
			}
		}

		// If msg arrive from Inputs
		else if (msg->arrivedOn("fromCPU")){
			for (i=0; i<numCPUs; i++)
				if (msg->arrivedOn ("fromCPU", i))
					return (gate("toCPU", i));
		}

	// If gate not found!
	return nullptr;
}


void CPU_Scheduler_RR::processSelfMessage (cMessage *msg){
	showErrorMessage ("Unknown self message [%s]", msg->getName());
}


void CPU_Scheduler_RR::processRequestMessage (Packet * pkt){

	int cpuIndex;
	//icancloud_App_CPU_Message *sm_cpu;

	
	//icancloud_Message *sm

		// Casting to debug!
	    pkt->trimFront();
		auto sm_cpu = pkt->removeAtFront<icancloud_App_CPU_Message>();
		
		// Assign infinite quantum
		sm_cpu->setQuantum(quantum);
				
		// Search for an empty cpu core
		cpuIndex = searchIdleCPU();
		
		// All CPUs are busy
		if (cpuIndex == NOT_FOUND){
			
			if (DEBUG_CPU_Scheduler_RR)
				showDebugMessage ("Enqueing computing block. All CPUs are busy: %s", sm_cpu->contentsToString(DEBUG_MSG_CPU_Scheduler_RR).c_str());
			
			// Enqueue current computing block
			pkt->insertAtFront(sm_cpu);
			requestsQueue.insert (pkt);
		}
		
		// At least, one cpu core is idle
		else{
			
			if (DEBUG_CPU_Scheduler_RR)
				showDebugMessage ("Sending computing block to CPU[%d]:%s", cpuIndex, sm_cpu->contentsToString(DEBUG_MSG_CPU_Scheduler_RR).c_str());
			
			// Assign cpu core
			sm_cpu->setNextModuleIndex(cpuIndex);			
			
			// Update state!
			isCPU_Idle[cpuIndex]=false;
			pkt->insertAtFront(sm_cpu);
			sendRequestMessage (pkt, toCPUGate[cpuIndex]);
		}
}


void CPU_Scheduler_RR::processResponseMessage (Packet *pkt){

	unsigned int cpuIndex;	
	//Packet* unqueuedMessage;
	//icancloud_Message *nextRequest;
	//icancloud_App_CPU_Message *sm_cpu;
	//icancloud_App_CPU_Message *sm_cpuNext;
	
		// Cast
	pkt->trimFront();
	auto sm = pkt->removeAtFront<icancloud_Message>();
	auto sm_cpu = dynamicPtrCast<icancloud_App_CPU_Message>(sm);
    if (sm_cpu == nullptr)
        throw cRuntimeError("Header type error,  not of the type icancloud_App_CPU_Message");
	// Update cpu state!
	cpuIndex = sm_cpu->getNextModuleIndex();
	if ((cpuIndex >= numCPUs) || (cpuIndex < 0))
	    showErrorMessage ("CPU index error (%d). There are %d CPUs attached. %s\n",
	            cpuIndex,
	            numCPUs,
	            sm->contentsToString(true).c_str());
	else
		    isCPU_Idle[cpuIndex] = true;


	// Current computing block has not been completely executed
	if ((sm_cpu->getRemainingMIs() > 0) || (sm_cpu->getCpuTime() > 0.0000001)){
	    sm_cpu->setIsResponse(false);
	    pkt->insertAtFront(sm_cpu);
        requestsQueue.insert(pkt);
    } else {
        if (DEBUG_CPU_Scheduler_RR)
            showDebugMessage("Computing block finished! Sending back to App:%s",
                    sm_cpu->contentsToString(DEBUG_MSG_CPU_Scheduler_RR).c_str());

        sm_cpu->setIsResponse(true);
        pkt->insertAtFront(sm_cpu);
        sendResponseMessage(pkt);
    }

    // There are pending requests
    if (!requestsQueue.isEmpty()) {

        // Pop
        auto unqueuedMessage = (Packet *) requestsQueue.pop();

        // Dynamic cast!
        unqueuedMessage->trimFront();
        auto nextRequest = unqueuedMessage->removeAtFront<icancloud_Message>();
        // set the cpu for the return
        nextRequest->setNextModuleIndex(cpuIndex);

        // Update state!
        isCPU_Idle[cpuIndex] = false;

        auto sm_cpuNext = dynamicPtrCast<icancloud_App_CPU_Message>(nextRequest);
        if (sm_cpuNext == nullptr)
            throw cRuntimeError("Header type error,  not of the type icancloud_App_CPU_Message");


        // Debug
        if (DEBUG_CPU_Scheduler_RR)
            showDebugMessage("Sending computing block to CPU[%d]:%s", cpuIndex,
                    sm_cpuNext->contentsToString(DEBUG_MSG_CPU_Scheduler_RR).c_str());
        unqueuedMessage->insertAtFront(sm_cpuNext);
        // Send!
        sendRequestMessage(unqueuedMessage, toCPUGate[cpuIndex]);
    }
}


int CPU_Scheduler_RR::searchIdleCPU (){
	
	unsigned int i;
	bool found;
	int result;
	
		// Init
		i = 0;
		found = false;
	
		// Search for an idle CPU
		while ((i<numCPUs) && (!found)){			
			
			if (isCPU_Idle[i])
				found = true;
			else
				i++;
		}
				
		// Result
		if (found)
			result = i;
		else
			result = NOT_FOUND;		
	
	return result;
}


} // namespace icancloud
} // namespace inet
