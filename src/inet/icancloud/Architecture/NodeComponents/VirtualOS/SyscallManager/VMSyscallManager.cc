#include "inet/icancloud/Architecture/NodeComponents/VirtualOS/SyscallManager/VMSyscallManager.h"

namespace inet {

namespace icancloud {


Define_Module (VMSyscallManager);


VMSyscallManager::~VMSyscallManager(){
	
}

void VMSyscallManager::initialize(int stage){
    AbstractSyscallManager::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        mControllerPtr = dynamic_cast<VmMsgController*> (this->getParentModule()->getSubmodule("vmMsgController"));

        // Init the super-class

}

void VMSyscallManager::finish(){

	// Finish the super-class
    AbstractSyscallManager::finish();
}

void VMSyscallManager::processRequestMessage (Packet *pktSm){

	// if the msg comes from the node, the vmID has not setted by the msg controller
	// it will be marked as node, to the hypervisor
    pktSm->trim();
    const auto &sm = pktSm->peekAtFront<icancloud_Message>();
	int id;
	id = sm->getUid();
	int operation;

	if (id == -1)
	    throw cRuntimeError("processRequestMessage of VMSyscallManager-> message without id\n");

	operation = sm->getOperation();

    const auto &msg_mpi = dynamicPtrCast<const icancloud_MPI_Message_Base> (sm);


	// Msg cames from Network
	if (pktSm->getArrivalGate() == fromNetGate){
        bool found = false;

        if (msg_mpi == nullptr){
            sendRequestMessage (pktSm, toAppGates->getGate(sm->getNextModuleIndex()));
        }
        else {
            for (cModule::SubmoduleIterator i(getParentModule()); !i.end() && !found; i++) {
                cModule* currentApp = *i;
                if (strcmp(currentApp->getFullName(), "app" ) == 0) {// if submod() is in the same vector as this module
                    if (currentApp->hasPar("myRank")){
                        int currentRank = currentApp->par("myRank");

                        if ((int)msg_mpi->getDestRank() == currentRank){

                            for (int j = 0; (j < (int)processesRunning.size()) && !found;j++){
                                if ((*(processesRunning.begin() + j)) -> process->getId() == currentApp->getId()){
                                    sendRequestMessage(pktSm, toAppGates->getGate((*(processesRunning.begin() + j))->toGateIdx));
                                    found = true;
                                }
                            }
                        }
                    }
                }
            }
        }
	}
	
	// Msg cames from CPU
	else if (pktSm->getArrivalGate() == fromCPUGate){
		showErrorMessage ("This module cannot receive request messages from CPU!!!");
	}
	// Msg cames from Memory
	else if (pktSm->getArrivalGate() == fromMemoryGate){
		sendRequestMessage (pktSm, toAppGates->getGate(sm->getNextModuleIndex()));
	}	
	
	// Msg cames from applications
	else{
		
		// I/O operation?
		if ((operation == SM_OPEN_FILE)   	 	 	  ||
			(operation == SM_CLOSE_FILE)  	  		  ||
			(operation == SM_READ_FILE)   			  ||
			(operation == SM_WRITE_FILE)  			  ||
			(operation == SM_CREATE_FILE) 	 		  ||
			(operation == SM_DELETE_FILE) 			  ||
			(operation == SM_SET_HBS_TO_REMOTE)		  ||
			(operation == SM_DELETE_USER_FS) 	 		  ||
			(operation == SM_NOTIFY_USER_FS_DELETED) 	  ||
			(operation == SM_NOTIFY_PRELOAD_FINALIZATION)){

				// Remote operation? to NET
				if (sm->getRemoteOperation()){
					sendRequestMessage (pktSm, toNetGate);
				}
				
				// Local operation? to local FS
				else{
                    sendRequestMessage (pktSm, toMemoryGate);
				}
		// Set ior, a new vm is being allocated ..
		}
		else if(operation == SM_SET_IOR){
		    pktSm->trimFront();
		    auto  sm = pktSm->removeAtFront<icancloud_Message>();
			auto sm_io = CHK(dynamicPtrCast<icancloud_App_IO_Message>(sm));

			// Dup and send the request to create the vm into the local net manager
			auto sm_net = makeShared<icancloud_App_NET_Message>();
			sm_net->setUid(sm->getUid());
			sm_net->setPid(sm->getPid());
			sm_net->setOperation(sm->getOperation());
			sm_net->setLocalIP(sm_io->getNfs_destAddress());
			sm_io->setNfs_destAddress(nullptr);
            auto pktNet = new Packet("icancloud_App_NET_Message");
            pktNet->insertAtFront(sm_net);

            pktSm->insertAtFront(sm_io);

			sendRequestMessage (pktNet, toNetGate);
			// Send request to create the ior in the fs
			sendRequestMessage (pktSm, toMemoryGate);

		}
		
		// MPI operation?
		else if ((operation == MPI_SEND) ||
				 (operation == MPI_RECV) ||
				 (operation == MPI_BARRIER_UP)   ||
				 (operation == MPI_BARRIER_DOWN) ||
				 (operation == MPI_BCAST)   ||
				 (operation == MPI_SCATTER) ||
				 (operation == MPI_GATHER)){
			
			sendRequestMessage (pktSm, toNetGate);
		}
		
		
		// CPU operation?
		else if (operation == SM_CPU_EXEC) {

			sendRequestMessage (pktSm, toCPUGate);
		}

		// MEM operation?
		else if ((operation == SM_MEM_ALLOCATE) || (operation == SM_MEM_RELEASE)){

			sendRequestMessage (pktSm, toMemoryGate);
		}


		// Net operation?		
		else if ((operation == SM_CREATE_CONNECTION) || (operation == SM_LISTEN_CONNECTION) ||
				(operation == SM_SEND_DATA_NET)){

			sendRequestMessage (pktSm, toNetGate);
		}			

		else if ((operation == SM_CHANGE_DISK_STATE) || (operation == SM_CHANGE_CPU_STATE)|| (operation == SM_CHANGE_MEMORY_STATE) || (operation == SM_CHANGE_NET_STATE)){
		    delete(pktSm);
		}

		// Unknown operation! -> Error!!!
		else {
		    const auto &sm = pktSm->peekAtFront<icancloud_Message>();
			showErrorMessage ("Unknown operation:%i %s",sm->getOperation(), sm->operationToString().c_str());
		}
	}
}

int VMSyscallManager::createProcess(icancloud_Base* j, int uid){

    cModule* jobAppModule;
    UserJob* job;

    job = dynamic_cast <UserJob*>(j);

    if (job == nullptr) throw cRuntimeError("SyscallManager::createJob, error with dynamic casting. Entry parameter cannot cast to jobBase.\n");

    //get the app previously created
    jobAppModule = check_and_cast<cModule*>(job);
    jobAppModule->changeParentTo(getParentModule());

    //Connect the modules (app created and node selected)
    int newIndexFrom = fromAppGates->newGate("fromApps");
    int newIndexTo = toAppGates->newGate("toApps");

    mControllerPtr->linkNewApplication(jobAppModule,
            toAppGates->getGate(newIndexTo),
            fromAppGates->getGate(newIndexFrom));

    processRunning* proc;
    proc = new processRunning();
    proc->process = job;
    proc->uid = uid;
    proc->toGateIdx = newIndexTo;

    processesRunning.push_back(proc);

    return newIndexFrom;

}

void VMSyscallManager::removeProcess(int pId) {

    icancloud_Base* job = deleteJobFromStructures(pId);

    if (job != nullptr) {
        int position = mControllerPtr->unlinkApplication(job);
        fromAppGates->freeGate(position);
        toAppGates->freeGate(position);
    }

}

void VMSyscallManager::connectToRemoteStorage (vector<string> destinationIPs, string fsType, int localPort, int uId, int pId, string ipFrom, int jobId){
     throw cRuntimeError("VMSyscallManager::connectToRemoteStorage->trying to call this method from somewhere...\n");
}

void VMSyscallManager::createDataStorageListen (int uId, int pId){
    throw cRuntimeError("VMSyscallManager::createDataStorageListen->trying to call this method from somewhere...\n");
}

void VMSyscallManager::deleteUserFSFiles( int uId, int pId){
    throw cRuntimeError("VMSyscallManager::deleteUserFSFiles->trying to call this method from somewhere...\n");
}

void VMSyscallManager::setRemoteFiles (int uId, int pId, int spId, string ipFrom, vector<preload_T*> filesToPreload, vector<fsStructure_T*> fsPaths){
    throw cRuntimeError("VMSyscallManager::setRemoteFiles->trying to call this method from somewhere...\n");

}

void VMSyscallManager::createLocalFiles ( int uId, int pId, int spId, string ipFrom, vector<preload_T*> filesToPreload, vector<fsStructure_T*> fsPaths){
    throw cRuntimeError("VMSyscallManager::createLocalFiles->trying to call this method from somewhere...\n");
}

string VMSyscallManager::getState (){
    return state;
}

void VMSyscallManager::changeState(const string & newState){
    state = newState;
}

void VMSyscallManager::setManager(icancloud_Base* manager){
    user = check_and_cast<AbstractCloudUser*>(manager);
    mControllerPtr->setId(user->getUserId(), this->getParentModule()->getParentModule()->getId());
}

} // namespace icancloud
} // namespace inet
