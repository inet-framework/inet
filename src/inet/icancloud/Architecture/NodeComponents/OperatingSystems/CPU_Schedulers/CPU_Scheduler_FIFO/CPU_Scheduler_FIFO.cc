#include "CPU_Scheduler_FIFO.h"

namespace inet {

namespace icancloud {


Define_Module (CPU_Scheduler_FIFO);



CPU_Scheduler_FIFO::~CPU_Scheduler_FIFO(){
		
	requestsQueue.clear();			
}


void CPU_Scheduler_FIFO::initialize(int stage) {

    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        std::ostringstream osStream;
        unsigned int i;

        // Set the moduleIdName
        osStream << "CPU_Scheduler_FIFO." << getId();
        moduleIdName = osStream.str();

        // Init the super-class

        // Get module parameters
        numCPUs = par("numCPUs");

        // State of CPUs
        isCPU_Idle = new bool[numCPUs];

        // Init state to idle!
        for (i = 0; i < numCPUs; i++)
            isCPU_Idle[i] = true;

        // Init requests queue
        requestsQueue.clear();
        queueSize = 0;

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


void CPU_Scheduler_FIFO::finish(){

	// Finish the super-class
	icancloud_Base::finish();
}


cGate* CPU_Scheduler_FIFO::getOutGate(cMessage *msg) {

    unsigned int i;

    // If msg arrive from Output
    if (msg->getArrivalGate() == fromOsGate) {
        if (gate("toOsGate")->getNextGate()->isConnected()) {
            return (toOsGate);
        }
    }

    // If msg arrive from Inputs
    else if (msg->arrivedOn("fromCPU")) {
        for (i = 0; i < numCPUs; i++)
            if (msg->arrivedOn("fromCPU", i))
                return (gate("toCPU", i));
    }

    // If gate not found!
    return nullptr;
}


void CPU_Scheduler_FIFO::processSelfMessage (cMessage *msg){
	showErrorMessage ("Unknown self message [%s]", msg->getName());
}


void CPU_Scheduler_FIFO::processRequestMessage (Packet *pkt){

	int cpuIndex;

	int operation;

	const auto &sm = pkt->peekAtFront<icancloud_Message>();
	operation = sm->getOperation ();
	
	if (operation != SM_CHANGE_CPU_STATE){
		// Casting to debug!
	    pkt->trimFront();
	    auto sm = pkt->removeAtFront<icancloud_Message>();
		auto sm_cpu = dynamicPtrCast<icancloud_App_CPU_Message>(sm);
		if (sm_cpu == nullptr)
		    throw cRuntimeError("Header type error");
		
		// Assign infinite quantum
		sm_cpu->setQuantum(INFINITE_QUANTUM);
				
		// Search for an empty cpu core
		cpuIndex = searchIdleCPU();
		
		// All CPUs are busy
		if (cpuIndex == NOT_FOUND){
			
			if (DEBUG_CPU_Scheduler_FIFO)
				showDebugMessage ("Enqueing computing block. All CPUs are busy: %s", sm_cpu->contentsToString(DEBUG_MSG_CPU_Scheduler_FIFO).c_str());
			
			// Enqueue current computing block
			pkt->insertAtFront(sm_cpu);
			requestsQueue.insert (pkt);
			queueSize++;
		}
		
		// At least, one cpu core is idle
		else{
			
			if (DEBUG_CPU_Scheduler_FIFO)
				showDebugMessage ("Sending computing block to CPU[%d]:%s", cpuIndex, sm_cpu->contentsToString(DEBUG_MSG_CPU_Scheduler_FIFO).c_str());
			
			// Assign cpu core
			sm_cpu->setNextModuleIndex(cpuIndex);			
			pkt->insertAtFront(sm_cpu);
			
			// Update state!
			isCPU_Idle[cpuIndex]=false;		
			sendRequestMessage (pkt, toCPUGate[cpuIndex]);
		}
	}
	else {

		sendRequestMessage (pkt, toCPUGate[0]);

	}
}


void CPU_Scheduler_FIFO::processResponseMessage(Packet *pkt) {

    unsigned int cpuIndex;
    unsigned int size;
    pkt->trimFront();
    auto sm = pkt->removeAtFront<icancloud_Message>();
    auto sm_cpu = dynamicPtrCast < icancloud_App_CPU_Message > (sm);
    if (sm_cpu == nullptr)
        throw cRuntimeError("Header type error");

    cpuIndex = sm->getNextModuleIndex();

    // Update cpu state!
    if ((cpuIndex >= numCPUs) || (cpuIndex < 0))
        showErrorMessage("CPU index error (%d). There are %d CPUs attached. %s\n",
                cpuIndex, numCPUs, sm->contentsToString(true).c_str());
    else
        isCPU_Idle[cpuIndex] = true;

    if (DEBUG_CPU_Scheduler_FIFO)
        showDebugMessage("Computing block Finished. Sending back to app:%s",
                sm_cpu->contentsToString(DEBUG_MSG_CPU_Scheduler_FIFO).c_str());

    pkt->insertAtFront(sm_cpu);

    // There are pending requests
    if (!requestsQueue.isEmpty()) {

        // Calculate the number of requests enqueued

        size = requestsQueue.getLength();

        auto new_msg = makeShared<icancloud_App_CPU_Message>();
        new_msg->setOperation(SM_CHANGE_CPU_STATE);

        if (queueSize >= size) {

            new_msg->setChangingState(INCREMENT_SPEED);

        }
        else if (queueSize < size) {

            new_msg->setChangingState(DECREMENT_SPEED);

        }

        auto pkt_newMsg = new Packet("icancloud_App_CPU_Message");
        pkt_newMsg->insertAtBack(new_msg);

        sendRequestMessage(pkt_newMsg, toCPUGate[cpuIndex]);
        // Pop
        auto unqueuedMessage = (Packet *) requestsQueue.pop();
        queueSize--;
        // Dynamic cast!
        unqueuedMessage->trimFront();
        auto nextRequest = unqueuedMessage->removeAtFront<icancloud_Message>();

        // set the cpu for the return
        nextRequest->setNextModuleIndex(cpuIndex);

        // Update state!
        isCPU_Idle[cpuIndex] = false;

        auto sm_cpuNext = dynamicPtrCast<icancloud_App_CPU_Message>(nextRequest);
        if (sm_cpuNext == nullptr)
            throw cRuntimeError("Header type error");

        if (DEBUG_CPU_Scheduler_FIFO)
            showDebugMessage("Sending computing block to CPU[%d]:%s", cpuIndex,
                    sm_cpuNext->contentsToString(DEBUG_MSG_CPU_Scheduler_FIFO).c_str());

        // Send!
        unqueuedMessage->insertAtFront(nextRequest);
        sendRequestMessage(unqueuedMessage, toCPUGate[cpuIndex]);
    }
    else {
        auto new_msg = makeShared<icancloud_App_CPU_Message>();
        new_msg->setOperation(SM_CHANGE_CPU_STATE);
        // Set the corresponding parameters
        new_msg->setChangingState(MACHINE_STATE_IDLE);
        new_msg->add_component_index_To_change_state(cpuIndex);
        auto pktAux = new Packet("icancloud_App_CPU_Message");
        pktAux->insertAtFront(new_msg);
        sendRequestMessage(pktAux, toCPUGate[cpuIndex]);
    }

    // Send current response to OS
    sendResponseMessage(pkt);
}


int CPU_Scheduler_FIFO::searchIdleCPU (){
	
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
