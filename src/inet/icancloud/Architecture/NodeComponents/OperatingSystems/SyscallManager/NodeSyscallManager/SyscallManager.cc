#include "inet/icancloud/Architecture/NodeComponents/OperatingSystems/SyscallManager/NodeSyscallManager/SyscallManager.h"

namespace inet {

namespace icancloud {


Define_Module (SyscallManager);


SyscallManager::~SyscallManager(){
	
}


void SyscallManager::initialize(int stage) {

    // Init the super-class
    AbstractSyscallManager::initialize(stage);
    if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT) {
        statesAppPtr = nullptr;
        ioManager = nullptr;

        cModule* mod;

        mod = getParentModule()->getSubmodule("nodeStates");
        statesAppPtr = dynamic_cast<StatesApplication*>(mod);

        mod = getParentModule()->getSubmodule("remoteStorage");
        ioManager = dynamic_cast<RemoteStorageApp*>(mod);

        // Get module params

        remoteStorageGate = 0;
        statesAppGate = 1;

        for (int i = 0; i < 2; i++) {
            toAppGates->linkGate("toApps", i);
            fromAppGates->linkGate("fromApps", i);
        }
    }
}


void SyscallManager::finish(){

	// Finish the super-class
    AbstractSyscallManager::finish();
}


void SyscallManager::processRequestMessage (Packet *pkt){

	// if the msg comes from the node, the vmID has not setted by the msg controller
	// it will be marked as node, to the hypervisor

    pkt->trim();
    auto sm = pkt->removeAtFront<icancloud_Message>();


	int pid = sm->getPid();
	int uid = sm->getUid();

	int operation;

	// Label the message. It comes from the OS
	if (uid == -1)
	    sm->setUid(0);
	if (pid == -1)
	    sm->setPid(0);

	operation = sm->getOperation();

	pkt->insertAtFront(sm);

	// Msg cames from Network
	if (pkt->getArrivalGate() == fromNetGate){
		
		if ((operation == SM_VM_ACTIVATION) ||
			(operation == SM_ITERATIVE_PRECOPY) ||
			(operation == SM_STOP_AND_DOWN_VM)){

			sendRequestMessage (pkt, toAppGates->getGate(remoteStorageGate));

		}else {

			sendRequestMessage (pkt, toAppGates->getGate(sm->getNextModuleIndex()));

		}
	}
	
	// Msg cames from CPU
	else if (pkt->getArrivalGate() == fromCPUGate){
		showErrorMessage ("This module cannot receive request messages from CPU!!!");
	}
	
	
	// Msg cames from Memory
	else if (pkt->getArrivalGate() == fromMemoryGate){
			
		sendRequestMessage (pkt, toAppGates->getGate(sm->getNextModuleIndex()));
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
			(operation == SM_CHANGE_DISK_STATE)		  ||
			(operation == SM_SET_HBS_TO_REMOTE)		  ||
			(operation == SM_DELETE_USER_FS)
			){

				// Remote operation? to NET
				if (sm->getRemoteOperation()){
					
					unsigned int aux = remoteStorageGate;
					if (sm->getNextModuleIndex() == aux){
						sendRequestMessage (pkt, toAppGates->getGate(remoteStorageGate));
					}
					else{
						sendRequestMessage (pkt, toNetGate);
					}

				}
				
				// Local operation? to local FS
				else{
					sendRequestMessage (pkt, toMemoryGate);
				}
		// Set ior, a new vm is being allocated ..
		} else if(operation == SM_SET_IOR){
		    pkt->trimFront();
		    auto sm = pkt->removeAtFront<icancloud_Message>();
			auto  sm_io = dynamicPtrCast<icancloud_App_IO_Message>(sm);

			if (sm_io == nullptr)
			    throw cRuntimeError("Not of type icancloud_App_IO_Message");

			// Dup and send the request to create the vm into the local net manager
			auto sm_net = makeShared<icancloud_App_NET_Message>();

			sm_net->setUid(sm->getUid());
			sm_net->setPid(sm->getPid());
			sm_net->setOperation(sm->getOperation());
			sm_net->setLocalIP(sm_io->getNfs_destAddress());

			auto pkt2 = new Packet("icancloud_App_NET_Message");

			pkt2->insertAtFront(sm_net);



			sendRequestMessage (pkt2, toNetGate);

			// Send request to create the ior in the fs
			sm_io->setNfs_destAddress(nullptr);
			pkt->insertAtFront(sm_io);
			sendRequestMessage (pkt, toMemoryGate);

		}
		
		// MPI operation?
		else if ((operation == MPI_SEND) ||
				 (operation == MPI_RECV) ||
				 (operation == MPI_BARRIER_UP)   ||
				 (operation == MPI_BARRIER_DOWN) ||
				 (operation == MPI_BCAST)   ||
				 (operation == MPI_SCATTER) ||
				 (operation == MPI_GATHER)){
			
			sendRequestMessage (pkt, toNetGate);
		}
		
		
		// CPU operation?
		else if ((operation == SM_CPU_EXEC) || (operation == SM_CHANGE_CPU_STATE)) {

			sendRequestMessage (pkt, toCPUGate);
		}

		// MEM operation?
		else if ((operation == SM_MEM_ALLOCATE) || (operation == SM_MEM_RELEASE) ||	 (operation == SM_CHANGE_MEMORY_STATE)){

			sendRequestMessage (pkt, toMemoryGate);
		}


		// Net operation?		
		else if ((operation == SM_CREATE_CONNECTION) || (operation == SM_LISTEN_CONNECTION) ||
				(operation == SM_SEND_DATA_NET) || (operation == SM_CHANGE_NET_STATE)){

			sendRequestMessage (pkt, toNetGate);
		}			
		
		// Remote Storage or migration operation?
		else if (
				(operation == SM_VM_REQUEST_CONNECTION_TO_STORAGE) || (operation == SM_NODE_REQUEST_CONNECTION_TO_MIGRATE) ||
				(operation == SM_MIGRATION_REQUEST_LISTEN) || (operation == SM_UNBLOCK_HBS_TO_REMOTE) ||
				(operation == SM_CLOSE_CONNECTION) || (operation == SM_CLOSE_VM_CONNECTIONS) ||
				(operation == SM_VM_ACTIVATION) || (operation == SM_ITERATIVE_PRECOPY) ||
				(operation == SM_STOP_AND_DOWN_VM) || (operation == SM_CONNECTION_CONTENTS)
				){

			sendRequestMessage (pkt, toNetGate);
		}

		// Set hypervisor contents (vm migration)
		else if (operation == SET_MIGRATION_CONNECTIONS){
		    pkt->trimFront();
		    auto sm = pkt->removeAtFront<icancloud_Message>();
		    auto sm_hbs_data = dynamicPtrCast<icancloud_Migration_Message> (sm);

			if (sm_hbs_data->getMemorySizeKB() != 0){
			    auto pkt2 = pkt->dup();

				auto sm_hmem_data = staticPtrCast<icancloud_Migration_Message>(sm_hbs_data->dupShared());
				sm_hmem_data->setOperation(ALLOCATE_MIGRATION_DATA);
				pkt2->insertAtFront(sm_hmem_data);
				sendRequestMessage (pkt2, toMemoryGate);
			}

			pkt->insertAtFront(sm_hbs_data);
			// In the net manager redirect to the bs manager.
			sendRequestMessage (pkt, toNetGate);

		}

		// Set memory and disk data (vm migration)
		else if (operation == ALLOCATE_MIGRATION_DATA){
		    pkt->trimFront();
            auto sm = pkt->removeAtFront<icancloud_Message>();
            auto sm_hbs_data = dynamicPtrCast<icancloud_Migration_Message> (sm);

			pkt->insertAtFront(sm);

			if (sm_hbs_data->getMemorySizeKB() != 0){
				sendRequestMessage (pkt, toMemoryGate);
			}

			if (sm_hbs_data->getDiskSizeKB() != 0){
				sendRequestMessage (pkt, toNetGate);
			}
		}

		// Get memory and disk/connections data (vm migration)
		else if ((operation == GET_MIGRATION_DATA) || (operation == GET_MIGRATION_CONNECTIONS)){
			sendRequestMessage (pkt->dup(), toMemoryGate);
			sendRequestMessage (pkt, toNetGate);

		}
		else if((operation == SM_NOTIFY_USER_FS_DELETED)||
		        (operation == SM_NOTIFY_PRELOAD_FINALIZATION) ||
		        (operation == SM_NOTIFY_USER_CONNECTIONS_CLOSED)
		        ){
		    notifyManager(pkt);
		    delete(pkt);
		}
		// Unknown operation! -> Error!!!
		else
			showErrorMessage ("Unknown operation:%i %s",sm->getOperation(), sm->operationToString().c_str());
	}

}

void SyscallManager::notifyManager (Packet *sms){

    nodePtr->notifyManager(sms);

}

void SyscallManager::removeProcess(int pid){

    icancloud_Base* job = deleteJobFromStructures(pid);

    if (job != nullptr){
        int gateIdx = job->gate("fromOS")->getPreviousGate()->getId();
        int position = toAppGates->searchGate(gateIdx);

        fromAppGates->freeGate(position);
        toAppGates->freeGate(position);

        job->callFinish();

    }

}

int SyscallManager::createProcess(icancloud_Base* job, int uid){

    if (job == nullptr) throw cRuntimeError("SyscallManager::createJob, error with dynamic casting. Entry parameter cannot cast to jobBase.\n");

    int newIndexFrom = fromAppGates->newGate("fromApps");
    int newIndexTo = toAppGates->newGate("toApps");

    //get the app previously created
    job->changeParentTo(this);

    //Connect the modules (app created and node selected)
        fromAppGates->connectIn(job->gate("fromOS"), newIndexFrom);
        toAppGates->connectOut(job->gate("toOS"), newIndexTo);

        processRunning* proc;
        proc = new processRunning();
        proc->process = job;
        proc->uid = uid;
        processesRunning.push_back(proc);

    return newIndexTo;

}

void SyscallManager::initializeSystemApps(int storagePort, const string & state){
    statesAppPtr->initState(state);
    // Initialize the local port if this is a storage node..
    if (storagePort != -1) ioManager -> initialize_storage_data (storagePort);

}

void SyscallManager::changeState(const string &newState){
    if (MACHINE_STATE_OFF == newState){
        resetSystem();
    }
    statesAppPtr->changeState(newState);
}


} // namespace icancloud
} // namespace inet
