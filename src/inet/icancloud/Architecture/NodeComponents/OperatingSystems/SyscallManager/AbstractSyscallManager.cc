#include "inet/icancloud/Architecture/NodeComponents/OperatingSystems/SyscallManager/AbstractSyscallManager.h"

namespace inet {

namespace icancloud {


AbstractSyscallManager::~AbstractSyscallManager(){
	
}


void AbstractSyscallManager::initialize(int stage) {

    // Init the super-class
    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

        pendingJobs.clear();
        processesRunning.clear();

        // Initialize
        std::ostringstream osStream;

        // Set the moduleIdName
        osStream << "." << getId();
        moduleIdName = osStream.str();

        toAppGates = new cGateManager(this);
        fromAppGates = new cGateManager(this);

        fromMemoryGate = gate("fromMemory");
        fromNetGate = gate("fromNet");
        fromCPUGate = gate("fromCPU");

        toMemoryGate = gate("toMemory");
        toNetGate = gate("toNet");
        toCPUGate = gate("toCPU");

        processesRunning.clear();

        totalMemory_KB = totalStorage_KB = memoryFree_KB = storageFree_KB = 0;
    }

}


void AbstractSyscallManager::finish(){

	// Finish the super-class
	icancloud_Base::finish();
}

cGate* AbstractSyscallManager::getOutGate (cMessage *msg){

        // If msg arrive from Applications
		if (msg->arrivedOn("fromApps")){
			return (gate("toApps", msg->getArrivalGate()->getIndex()));
		}

		// If msg arrive from Memory
		else if (msg->getArrivalGate()==fromMemoryGate){
			if (gate("toMemory")->getNextGate()->isConnected()){
				return (toMemoryGate);
			}
		}
		
		// If msg arrive from Net
		else if (msg->getArrivalGate()==fromNetGate){
			if (gate("toNet")->getNextGate()->isConnected()){
				return (toNetGate);
			}
		}
		
		// If msg arrive from CPU
		else if (msg->getArrivalGate()==fromCPUGate){
			if (gate("toCPU")->getNextGate()->isConnected()){
				return (toCPUGate);
			}
		}		
		
	// If gate not found!
	return nullptr;
}

void AbstractSyscallManager::processSelfMessage(cMessage* msg) {

    //icancloud_Message* hmsg;
    //hmsg = dynamic_cast<icancloud_Message*>(msg);

    auto pktHmsg = check_and_cast<Packet *>(msg);
    const auto &hmsg = pktHmsg->peekAtFront<icancloud_Message>();

    if (pktHmsg->isSelfMessage()) {
        bool found = false;

        for (int i = 0; (i < (int) pendingJobs.size()) && (!found); i++) {
            if ((*(pendingJobs.begin() + i))->messageId == pktHmsg->getId()) {

                found = true;
                createProcess((*(pendingJobs.begin() + i))->job,
                        hmsg->getUid());
                pendingJobs.erase(pendingJobs.begin() + i);
            }
        }

        if (!found)
            throw cRuntimeError(
                    "AbstractSyscallManager::processSelfMessage -> error. There is not pending jobs to be created\n");

        cancelAndDelete(msg);
    } else {
        throw cRuntimeError("Unknown message at AbstractNode::handleMessage\n");
    }
}

void AbstractSyscallManager::processResponseMessage (Packet  *pktSm) {

    // Send back the message
    sendResponseMessage (pktSm);
}

int AbstractSyscallManager::searchUserId(int jobId){
    bool found = false;
    int uid = -1;

    for (int i = 0; (i < (int)processesRunning.size()) && (!found); i++){
        if ((*(processesRunning.begin() + i))->process->getId() == jobId){
            found = true;
            uid = (*(processesRunning.begin() + i))->process->getId();
        }
    }

    if (uid == -1) throw cRuntimeError("SyscallManager::searchUserId- Job with id = %i has not allocated as processes running\n", jobId);

    return uid;
}

void AbstractSyscallManager::removeAllProcesses(){

        for (int i = 0; i < (int)processesRunning.size(); i++)
            removeProcess((*(processesRunning.begin() + i))->process->getId());
}

void AbstractSyscallManager::allocateJob(icancloud_Base* job, simtime_t timeToStart, int uId){

    // Define ..
        //icancloud_Message* msg;
        pendingJob* p_job;

        auto pktSm = new Packet("icancloud_Message process_job");
        auto msg = makeShared<icancloud_Message>();

        msg->setUid(uId);
        pktSm->insertAtFront(msg);


        // Creates the structure;
        p_job = new pendingJob();
        p_job->job = job;
        p_job->messageId = pktSm->getId();

        pendingJobs.push_back(p_job);

        scheduleAt (simTime()+timeToStart, pktSm);

}

bool AbstractSyscallManager::isAppRunning(int pId){

    bool found = false;

    for (int i = 0; (i < (int)processesRunning.size()) && (!found); i++){
        if ((*(processesRunning.begin() + i))->process->getId() == pId){
            found = true;
        }
    }

    if (!found){
        for (int i = 0; (i < (int)pendingJobs.size()) && (!found); i++){
            if ((*(pendingJobs.begin() + i))->job->getId() == pId){
                found = true;
            }
        }
    }

    return found;

}

icancloud_Base* AbstractSyscallManager::deleteJobFromStructures(int jobId){
    bool found = false;
    icancloud_Base* job;

    for (int i = 0; (i < (int)processesRunning.size()) && (!found); i++){
        if ((*(processesRunning.begin() + i))->process->getId() == jobId){
            found = true;
            job = (*(processesRunning.begin() + i))->process;
            processesRunning.erase(processesRunning.begin() + i);
        }
    }


    if (!found){
        for (int i = 0; (i < (int)pendingJobs.size()) && (!found); i++){
            if ((*(pendingJobs.begin() + i))->job->getId() == jobId){
                found = true;
                job = (*(pendingJobs.begin() + i))->job;
                pendingJobs.erase(pendingJobs.begin() + i);
            }
        }
    }

    return job;
}

} // namespace icancloud
} // namespace inet
