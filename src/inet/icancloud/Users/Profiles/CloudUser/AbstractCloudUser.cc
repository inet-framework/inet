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

#include "inet/icancloud/Users/Profiles/CloudUser/AbstractCloudUser.h"

namespace inet {

namespace icancloud {


AbstractCloudUser::~AbstractCloudUser() {

}

void AbstractCloudUser::initialize(int stage) {
    AbstractUser::initialize (stage);
    // Initialize structures and parameters
    if (stage == INITSTAGE_LOCAL) {
        vms_waiting_remote_st.clear();

        machinesMap = new MachinesMap();

        pendingVmsAtStartup = 0;
        pending_shutdown_requests = 0;

        configMPI = nullptr;

        jobToDelete.clear();
        wastedVMs.clear();
    }
}

void AbstractCloudUser::finish(){

        // Parameter to be used by userBehavior

            vmListToExecute.clear();
            vms_waiting_remote_st.clear();
            wastedVMs.clear();

            for (unsigned int i =0; i < pending_jobs.size();){
                pending_jobs[0]->callFinish();
                pending_jobs[0]->deleteModule();
            }
    AbstractUser::finish();
}

void AbstractCloudUser::processSelfMessage (cMessage *msg){
    delete(msg);

    for (int i = 0; i < (int)jobToDelete.size();) {
        (*(jobToDelete.begin()+i))->deleteModule();
        jobToDelete.erase(jobToDelete.begin()+i);
    }

    executePendingJobs();
    schedule();
}

bool AbstractCloudUser::finalizeUser (){

    return AbstractUser::finalizeUser();

}

void AbstractCloudUser::createVMSet (int vmsQuantity, int numCores, int memorySizeMB, int storageSizeGB, int numNetIF, string vmSelectionType){

    userVmType* vms;
    elementType* el;
    bool found = false;

    el = new elementType();

    for (int i = 0; (i < (int)vmsToBeSelected.size()) && (!found); i++){
        if ( (strcmp((*(vmsToBeSelected.begin() +i))->type->getType().c_str(),vmSelectionType.c_str()) == 0)){
            (*(vmsToBeSelected.begin() +i))->quantity += vmsQuantity;
            found = true;
        }
    }
    if (!found){
        el->setTypeParameters(numCores, memorySizeMB,storageSizeGB,numNetIF,vmSelectionType.c_str(),1);
        vms = new userVmType();
        vms->type = el;
        vms->quantity = vmsQuantity;
        vmsToBeSelected.push_back(vms);
    }
};

void AbstractCloudUser::increaseVMSet (int vmsQuantity, string vmType){

    bool found = false;

    for (int i = 0; (i < (int)vmsToBeSelected.size()) && (!found); i++){
        if ( (strcmp((*(vmsToBeSelected.begin() +i))->type->getType().c_str(),vmType.c_str()) == 0)){
            (*(vmsToBeSelected.begin() +i))->quantity += vmsQuantity;
            found = true;
        }
    }
    if (!found){
        throw cRuntimeError("AbstractCloudUser::increaseVMSet ->increasing an inexistent set %s", vmType.c_str());
    }

};

void AbstractCloudUser::decreaseVMSet (string vmType, int vmsQuantity){

    bool found = false;

    for (int i = 0; (i < (int)vmsToBeSelected.size()) && (!found); i++){
        if ( (strcmp((*(vmsToBeSelected.begin() +i))->type->getType().c_str(),vmType.c_str()) == 0)){
            (*(vmsToBeSelected.begin() +i))->quantity--;
            if ((*(vmsToBeSelected.begin() +i))->quantity < 0) (*(vmsToBeSelected.begin() +i))->quantity = 0;
            found = true;
        }
    }
    if (!found){
        throw cRuntimeError("AbstractCloudUser::increaseVMSet ->increasing an inexistent set %s", vmType.c_str());
    }


}

int AbstractCloudUser::machinesLeft (int index){

    if (index > (int)(vmsToBeSelected.size() -1)) throw cRuntimeError("AbstractCloudUser::machinesLeft->index %i major than vmsSelectedSize %i\n",index, vmsToBeSelected.size() );

    return (*(vmsToBeSelected.begin() +index))->quantity;;

}

void AbstractCloudUser::startVMs (AbstractRequest* vmSet){


    vmSet -> setOperation (REQUEST_START_VM);
    vmSet -> setUid (this->getId());

//
//    for (vmSetIt = vmSet.begin(); vmSetIt < vmSet.end(); ){
//        if (  ((*vmSetIt)->getState() == MACHINE_STATE_OFF) &&
//              ((*vmSetIt)->getPendingOperation() == NOT_PENDING_OPS)
//           ){
//            (*vmSetIt)->setPendingOperation(PENDING_STARTUP);
//            vmSetIt++;
//        } else {
//            vmSet.erase(vmSetIt);
//        }
//    }
//
//    req -> setVectorVM (vmSet);

    send_request_to_manager(vmSet);

}

void AbstractCloudUser::shutdown_VM (VM* vm){

    RequestVM* req;
    vector<VM*> vms;

    vms.clear();

    if (vm->getState() != MACHINE_STATE_OFF){

        pending_shutdown_requests ++;

        vm->setPendingOperation(PENDING_SHUTDOWN);

        vms.push_back(vm);

        req = new RequestVM();
        req -> setOperation (REQUEST_FREE_RESOURCES);
        req -> setUid (this->getId());
        req -> setPid (vm->getPid());
        req -> setVectorVM(vms);

        send_request_to_manager(req);

    }
}

bool AbstractCloudUser::checkAllVMShutdown(){

    bool ok;
    int count1, count2;
    unsigned int i;

    ok = true;

    for (i = 0; (i < machinesMap->size()) && ok; i++){
        count1 = machinesMap->countOFFMachines(i);
        count2 = machinesMap->getSetQuantity(i);
        if (count1 != count2){
            ok = false;
        }
    }

    return ok;
}

int AbstractCloudUser::allocateJob(jobBase* job){

    // Define ..
        cModule* syscallManager;
        cModule* osModule;
        VMSyscallManager* osCore;
        Machine* vm;
        UserJob* jobC;
        int commId;

    // Initialize..
        jobC = check_and_cast<UserJob*>(job);
        vm = jobC->getMachine();

        if (vm == nullptr) throw cRuntimeError ("User profile has allocate the VM at the job before call createFS.\n");

    // Begin ..
        osModule = vm->getSubmodule("osModule");

        // Get AppModule vector
           syscallManager = osModule->getSubmodule("syscallManager");

           osCore = check_and_cast <VMSyscallManager*> (syscallManager);

           commId =  osCore->createProcess(jobC, this->getId());


        return commId;
}

void AbstractCloudUser::deleteJobVM (VM* vm, UserJob* job){
    vm->removeProcess(job->getId());
}

void AbstractCloudUser::deleteAllJobsVM (VM* vm){
    if (vm->getState() != MACHINE_STATE_OFF)
        vm->removeAllProcesses();
}


void AbstractCloudUser::start_up_job_execution (VM* vmToExecute, UserJob* job, JobQueue* qSrc, JobQueue* qDst, int qDst_pos){

    // Define ...
        int jobId;
//      cModule* startExecution;
        Machine* vm;
       // vector<VM*>::iterator setVMit;
        jobBase* jobB;
        UserJob* jobAux;
        int index = 0;
        bool found = false;
    // Initialize...

        jobId = job->getId();

    //move from the qSrc to the scheduler queue of running jobs

        if (qSrc == nullptr){

            while ((index < waitingQueue->size()) && (!found)){

                jobB = waitingQueue->getJob(index);

             if (jobB->getId() == jobId){
                    found = true;
                    index++;
                }
            }

            if (qDst == nullptr){
                waitingQueue->move_to_qDst(index, runningQueue, qDst_pos);
            }
            else {
                waitingQueue->move_to_qDst(index, qDst, qDst_pos);
            }

        }else{


            while ((index < qSrc->size()) && (!found)){

                jobB = qSrc->getJob(index);

                if (jobB->getId() == jobId)
                    found = true;
                else
                    index++;
            }

            if (qDst == nullptr){
                qSrc->move_to_qDst (index, runningQueue, qDst_pos);
            }
            else {
                qSrc->move_to_qDst (index, qDst, qDst_pos);
            }

        }

        vm = job->getMachine();

        jobAux = dynamic_cast<UserJob*>(job);

       // Record the job data
        jobAux->setJob_startTime();

        // Start scope and app
            vm->changeState(MACHINE_STATE_RUNNING);
            jobAux->startExecution ();

}

void AbstractCloudUser::executePendingJobs(){

    // Define..
            UserJob* jobC;
            jobBase* jobB;
            VM* vm;
            int i,j;
            bool found = false;

        // Init ..
            i = 0;
        // Begin ..
            while (i < waiting_for_remote_storage_Queue->get_queue_size()){

                jobB = waiting_for_remote_storage_Queue->getJob(i);

                j = 0;
                found = false;

                jobC = dynamic_cast<UserJob*>(jobB);
                if (jobC == nullptr) throw cRuntimeError("AbstractCloudUser::executePendingJobs->jobB cannot be casted to CloudJob\n");

                vm = check_and_cast<VM*>(jobC->getMachine());

                if (vmHasStorageRequests(vm)){
                    found = true;
                }
                else{
                    j++;
                }


                if (!found){
                    start_up_job_execution (vm, jobC, waiting_for_remote_storage_Queue, runningQueue, runningQueue->get_queue_size());
                }
                else {
                    i++;
                }
            }
}

void AbstractCloudUser::notify_UserJobHasFinished (jobBase* job){

    string jobID;
    int wqs = getWQ_size();
    // Begin ..

    UserJob* jobC = check_and_cast<UserJob*>(job);

        /* Record the instant of job's finalization */
        job->setJob_endTime();

        setJobResults(job->getResults()->dup());

        /* User virtual method */
        jobHasFinished(job);

        //Finalize the job and move it to the finish queue
            moveFromRQ_toFQ(job->getJobId());

        // free the virtual machine
            VM* vm = check_and_cast<VM*>(jobC->getMachine());
            deleteJobVM(vm , jobC);



	if (wqs != 0) {

	    Enter_Method_Silent();

	    cMessage* msg;
	    msg = new cMessage();
        jobC->callFinish();
        jobToDelete.push_back(jobC);
	    scheduleAt(simTime(), msg);

	}



}



void AbstractCloudUser::notify_UserRequestAttendeed  (AbstractRequest* req){

	// Define ..
	//	vector<RequestVM*>::iterator reqIt;
		vector<VM*> vms_to_storage;
		int i,j;
		int storageConnectionSize;
		bool vm_erased;
		vector<VM*> vmToNotify;
		RequestVM* reqVM;
		VM* vm;
		Machine* mac;
		StorageRequest* stReq;
	// Initialize ..
		vm_erased = false;
		i = 0;
		vms_to_storage.clear();
		vmToNotify.clear();

	/*
	 * -------------------------------------------------------------
	 * --------------- PRE-CALL -> requestAttended -----------------
	 * -------------------------------------------------------------
	 */
    reqVM = dynamic_cast<RequestVM*>(req);
    stReq = dynamic_cast<StorageRequest*>(req);

	if (req->getOperation() == REQUEST_START_VM){

        vector<Machine*> machines;

        // cast to machines
        for (j = 0; j < (int)reqVM->getVectorVM().size();j++){

            vm = reqVM->getVM(j);
            vm->setManager(this);

            Machine* machine;
            machine = dynamic_cast<Machine*>(vm);
            machines.push_back(machine);
        }

        // Set the new VMs at map
       machinesMap->increaseInstances(machines, reqVM->getVM(0)->getTypeName());

		requestAttended (req);
	}
	else if (req->getOperation() == REQUEST_FREE_RESOURCES){
		pending_shutdown_requests --;

		 for (j = 0; j < (int)reqVM->getVectorVM().size();j++){
		     vm = reqVM->getVM(j);
		     machinesMap->destroyInstance(vm->getTypeName(), req->getPid());

		     wastedVMs.push_back(vm);
		     vm->callFinish();

		 }


		requestAttended (req);
	}
	else if (req->getOperation() == REQUEST_REMOTE_STORAGE){

	    if (stReq == nullptr) throw cRuntimeError ("AbstractCloudUser::notify_UserRequestAttendeed -> cannot cast to storage request\n");

		    mac = machinesMap->getMachineById(req->getPid());
		    vm = dynamic_cast<VM*>(mac);
	        if (vm == nullptr) throw cRuntimeError ("AbstractCloudUser::notify_UserRequestAttendeed -> cannot cast to VM from Machine\n");

			// Delete from the control structure
			vm_erased = eraseVMFromControlVector(vm, REQUEST_REMOTE_STORAGE);
			if (!vm_erased) showErrorMessage("Trying to erase a vm from vms_waiting_for_remote_storage vector which not exists (User:%i, vm:%s[%i]",vm->getUid(), vm->getTypeName().c_str(), vm->getPid());

			// Get the module and initialize it!
				vm->changeState(MACHINE_STATE_IDLE);
		        vm->setPendingOperation(NOT_PENDING_OPS);

		        // notify to the user the vm loaded for executing jobs..
		            reqVM = new RequestVM();
                    reqVM->setOperation(REQUEST_REMOTE_STORAGE);
		            vmToNotify.push_back(vm);
		            reqVM->setVectorVM(vmToNotify);

		            requestAttended (reqVM);

		            delete(reqVM);

	}else if (req->getOperation() == REQUEST_LOCAL_STORAGE){

        stReq = dynamic_cast<StorageRequest*>(req);

        if (stReq == nullptr) throw cRuntimeError ("AbstractCloudUser::notify_UserRequestAttendeed -> cannot cast to storage request\n");

		storageConnectionSize = stReq->getConnectionSize();

		for (i = 0; i < storageConnectionSize; i++){
            mac = machinesMap->getMachineById(req->getPid());
            vm = dynamic_cast<VM*>(mac);
            if (vm == nullptr) throw cRuntimeError ("AbstractCloudUser::notify_UserRequestAttendeed -> cannot cast to VM from Machine\n");

			// Delete from the control structure
			vm_erased = eraseVMFromControlVector(vm, REQUEST_LOCAL_STORAGE);
			if (!vm_erased) showErrorMessage("Trying to erase a vm from vms_waiting_for_remote_storage vector which not exists (User:%i, vm:%s[%i]",vm->getUid(), vm->getTypeName().c_str(), vm->getPid());

			if (!vmHasStorageRequests(vm)){
				vm->changeState(MACHINE_STATE_IDLE);
		        vm->setPendingOperation(NOT_PENDING_OPS);
			}

            // notify to the user the vm loaded for executing jobs..
                reqVM = new RequestVM();
                reqVM->setOperation(REQUEST_REMOTE_STORAGE);
                vmToNotify.push_back(vm);
                reqVM->setVectorVM(vmToNotify);

                requestAttended (reqVM);

                delete(reqVM);

		}
	}

	/*
	 * -------------------------------------------------------------
	 * --------------- POST-CALL -> requestAttended ----------------
	 * -------------------------------------------------------------
	 */
		// Begin ..
		if ((req->getOperation() == REQUEST_START_VM) && (!isEmpty_WQ())){


		    reqVM = dynamic_cast<RequestVM*>(req);

	        if (reqVM == nullptr) throw cRuntimeError ("AbstractCloudUser::notify_UserRequestAttendeed -> cannot cast to RequestVM\n");


			// Select the vms for request remote storage!
				while (reqVM->getNumberVM() != 0){

					// get the first vm form the request
						vm = reqVM->getVM(0);

					// Delete from the control structure
						vm_erased = eraseVMFromControlVector(vm, REQUEST_START_VM);
						if (!vm_erased) throw cRuntimeError("DEBUG_CLOUD_USE->Trying to erase a vm from vms_to_initialize vector which not exists (User:%i, vm:%s[%i]",vm->getUid(), vm->getTypeName().c_str(), vm->getPid());

					// Start the vms
                        vm->changeState(MACHINE_STATE_IDLE);
                        vm->setPendingOperation(NOT_PENDING_OPS);

					// Delete from the request
						reqVM->eraseVM(0);

				}
		}

		delete(req);

	if (!userFinalizing)  schedule();
}

bool AbstractCloudUser::hasPendingRequests(){
    return !((vms_waiting_remote_st.size() == 0) && (pending_shutdown_requests == 0));
}

vector<StorageRequest*> AbstractCloudUser::createFSforJob(jobBase* job, string opIp, string nodeSetName, int nodeId, int optionalID){

    return userStorage::createFSforJob(job, opIp, nodeSetName, nodeId, optionalID);

}

bool AbstractCloudUser::eraseVMFromControlVector(VM* vm, int operation){

	bool found;
	found = false;
	VM* vm_;
	unsigned int i = 0;

	if (operation == REQUEST_START_VM) {

	    pendingVmsAtStartup = pendingVmsAtStartup - 1;
	    found = true;

	    if (pendingVmsAtStartup < 0) found = false;

	} else if ((operation == REQUEST_REMOTE_STORAGE) ||
			  (operation == REQUEST_LOCAL_STORAGE)) {

		while ((!found) && (i < vms_waiting_remote_st.size())){

			vm_ = *(vms_waiting_remote_st.begin() + i);

			if ((strcmp (vm_->getTypeName().c_str(), vm->getTypeName().c_str()) == 0) &&
				(vm_->getPid() == vm->getPid())){

				found = true;
				vms_waiting_remote_st.erase(vms_waiting_remote_st.begin() + i);

			}

			i++;

		}

	} else {
		showErrorMessage ("Error erasing vm from control vector. Operation unknown");
	}

	return found;
}

bool AbstractCloudUser::vmHasStorageRequests(VM* vm){

	bool found;
	found = false;
	VM* vm_;
	unsigned int i = 0;


	while ((!found) && (i < vms_waiting_remote_st.size())){

		vm_ = *(vms_waiting_remote_st.begin() + i);

		if ((strcmp (vm_->getTypeName().c_str(), vm->getTypeName().c_str()) == 0) &&
			(vm_->getPid() == vm->getPid())){

			found = true;
		}

		i++;

	}

	return found;
}

int AbstractCloudUser::vms_waiting_for_remote_storage(){
	return vms_waiting_remote_st.size();
}

int AbstractCloudUser::vms_waiting_for_initialization(){
	return pendingVmsAtStartup;
}

VM* AbstractCloudUser::searchVmByPid(int pid){
    Machine* machine = nullptr;
    VM* vm = nullptr;
    bool found = false;

    for (int i = 0; (i < machinesMap->getMapQuantity()) && (!found); i++){
        for (int j = 0; (j < machinesMap->getSetQuantity(i)) && (!found); j++){

            machine = machinesMap->getMachineByIndex(i,j);

            if (machine->getId() == pid){
                vm = check_and_cast<VM*>(machine);
                found = true;
            }
        }
    }
    return vm;
}


void AbstractCloudUser::send_request_to_manager (AbstractRequest* req){

	VM* vm;
	Machine* machine;

	RequestVM* request;
	StorageRequest* req_st;

	request = dynamic_cast<RequestVM*>(req);
	req_st =  dynamic_cast<StorageRequest*>(req);

	if ((request == nullptr)  && (req_st == nullptr)) throw cRuntimeError("AbstractCloudUser::send_request_to_manager->error during casting to RequestVM or StorageRequest\n");

	if (req->getOperation() == REQUEST_START_VM){

	    for (int  i = 0; i < request->getDifferentTypesQuantity(); i++){
	        pendingVmsAtStartup = pendingVmsAtStartup + request->getSelectionQuantity(i);
	    }

	} else if ( (req->getOperation() == REQUEST_REMOTE_STORAGE) ||
			    (req->getOperation() == REQUEST_LOCAL_STORAGE)  ){

			machine = machinesMap->getMachineById(req->getPid());
			vm = check_and_cast<VM*>(machine);

			vm->setPendingOperation(PENDING_STORAGE);
			vms_waiting_remote_st.push_back(vm);

	} else if (req->getOperation() == REQUEST_FREE_RESOURCES){

		for (int i = 0; i < request->getNumberVM(); i++){
//            machine = machinesMap->getMachineById(req->getPid());
//            vm = check_and_cast<VM*>(machine);

			vm = request->getVM(i);
			vm->setPendingOperation(PENDING_SHUTDOWN);
		}
	}

	 managerPtr->userSendRequest(req);


}

void AbstractCloudUser::deleteAllVMs(){

    int size = wastedVMs.size();

    while(size != 0){
        VM* vm;
        vm = (*(wastedVMs.begin()));
        //delete(vm);
        size --;
    }
}

} // namespace icancloud
} // namespace inet
