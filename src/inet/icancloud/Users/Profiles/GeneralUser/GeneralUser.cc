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

#include "GeneralUser.h"

namespace inet {

namespace icancloud {


Define_Module(GeneralUser);

GeneralUser::~GeneralUser() {

	// TODO Auto-generated destructor stub
}

void GeneralUser::initialize(int stage){

    AbstractCloudUser::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        printVMs = par("printVMs").boolValue();
        printJobs = par("printJobs").boolValue();
        print = false;
    }
}

void GeneralUser::finish(){
    AbstractCloudUser::finish();
}


void GeneralUser::userInitialization(){

	// Define ..
		AbstractRequest* vms;

	// Select the vms to start (all) and then send the request to the cloud manager.
		vms = selectVMs_ToStartUp ();

	// Start all vms
		if (vms != nullptr)
		    startVMs(vms);
}

void GeneralUser::userFinalization (){

    // Define ..
        int finishQSize,i,k;
        string jobs;
        ostringstream userData;
        ostringstream userJobsData;
        ostringstream userVmsData;
        ofstream resFile;
        vmStatesLog_t* vm_state;
        vector<cModule*> mods;
        VM* vm;
    // Init ..

    // Begin..
        //Open the file
        if (print){
           resFile.open(file.c_str(), ios::app );
        }

    // Get the user data
        // UserName - startTime - totalTime -
        userData << userName.c_str() << ";" << startTime.dbl() << ";" << endTime.dbl() << endl;

        if (printJobs){
        // Get the jobs
            finishQSize = getJobResultsSize();

            for (i = 0; i < finishQSize; i++){
                jobs = printJobResults (getJobResults(i));
                userJobsData << jobs;
            }
        }

     // Get the vms
        if (printVMs){

            int size = wastedVMs.size();

            for (i = 0; i < size; i++){

                    vm = (*(wastedVMs.begin()));

                    userVmsData << "#" << vm->getTypeName() << "[" << vm->getNameString() << "]:";

                    for (k = 0; k < vm->getLogStatesQuantity(); k++){
                        vm_state = vm->getLogState(k);

                        if (vm_state->vm_state == MACHINE_STATE_OFF)
                            userVmsData << (vm_state->init_time_M) << ";";
                        userVmsData << (vm_state->vm_state) << ":" ;
                        userVmsData << (vm_state->init_time_M) << ";";
                    }

            }
            userVmsData << endl;
        }

    //Print everything and close the file
    if (print){
         resFile << userData.str().c_str();
         if (printJobs) resFile << userJobsData.str().c_str();
         if (printVMs) resFile << userVmsData.str().c_str();
         resFile << endl;                       // Put a blank line between users
         resFile.close();
     }

}
/* --------------------------------------------------------------------
 * ------------------------ Interact with VMs -------------------------
 * --------------------------------------------------------------------
 */

AbstractRequest* GeneralUser::selectVMs_ToStartUp (){

	//Define...
		unsigned int i;
		unsigned int setSize;
		bool reqFilled;
		int maxNumVMsToRequest;
		int size;
		RequestVM* req ;

	//Initialize...
		size = 0;
		maxNumVMsToRequest = getWQ_size();
		setSize = vmsToBeSelected.size();
		req = new RequestVM();
		reqFilled = false;

	/*
	 *  Obtain all the VMs of the user. Initially all are into VM_NOT_REQUESTED state.
	 *  Then, user request for the VMs change state to free, and that is task of the cloud manager.
	 *  If user request a VM that is running, the cloud manager discard that request.
	 */
	for (i = 0; (i != setSize) && (maxNumVMsToRequest != 0);i++){
	    size = (((*(vmsToBeSelected.begin() + i))->quantity) - maxNumVMsToRequest);
	    if (size <= 0){
	        req->setNewSelection((*(vmsToBeSelected.begin() + i))->type->getType(),
	                             (*(vmsToBeSelected.begin() + i))->quantity);

	        (*(vmsToBeSelected.begin() + i))->quantity = (*(vmsToBeSelected.begin() + i))->quantity - size;
	        reqFilled = true;
	    } else {
	        req->setNewSelection((*(vmsToBeSelected.begin() + i))->type->getType(), (*(vmsToBeSelected.begin() + i))->quantity - size);
	        reqFilled = true;
            (*(vmsToBeSelected.begin() + i))->quantity = (*(vmsToBeSelected.begin() + i))->quantity - maxNumVMsToRequest;
	        break;
	    }

	}

	if (!reqFilled) req = nullptr;

	AbstractRequest* reqA;
	reqA = dynamic_cast<AbstractRequest*>(req);

	return reqA;

}

AbstractRequest* GeneralUser::selectResourcesJob (jobBase* job){

	// Define ..
		VM* vm;
		Machine* machine;
		vector<VM*> selectedVMs;
		bool found;
		unsigned int i = 0;
		unsigned int j = 0;
		AbstractRequest* aReq;
		RequestVM* reqVM;
	// Init...
		found = false;
		vm = nullptr;
		found = false;
		selectedVMs.clear();
		reqVM = new RequestVM();

	// The behavior of the selection of VMs is to get the first VM in free state.
	for (i = 0; i < machinesMap->size() && (!found); i++){

		for (j = 0; (j < (unsigned int)machinesMap->getSetQuantity(i)) && (!found); j++){

		    machine = machinesMap->getMachineByIndex(i,j);
		    vm = dynamic_cast<VM*>(machine);
			if (vm->getPendingOperation() == NOT_PENDING_OPS){
                if (vm->getState() == MACHINE_STATE_IDLE) {
                //if ((vm->getVmState() == MACHINE_STATE_IDLE) || (vm->getVmState() == MACHINE_STATE_RUNNING)) {
                    selectedVMs.insert(selectedVMs.begin(), vm);
                    //selectedVMs.insert(selectedVMs.begin(), vm);
                    found = true;

                }
			}
		}
	}

//		machine = machinesMap->getMachineByIndex(0,0);
//		vm = dynamic_cast<VM*>(machine);
//		selectedVMs.insert(selectedVMs.begin(), vm);

		reqVM->setVectorVM(selectedVMs);
		aReq = dynamic_cast<AbstractRequest*>(reqVM);

	return aReq;

}

void GeneralUser::requestAttended (AbstractRequest* req){

	// Define ..
		int exceededVMs;
		int i;
		RequestVM* reqVM;

	// Initialize ..
		i = 0;


	// Begin ..
	if (req->getOperation() == REQUEST_START_VM){

        reqVM = dynamic_cast<RequestVM*>(req);

		exceededVMs = reqVM->getVMQuantity() -  getWQ_size();

		if (isEmpty_WQ()){
			while (i !=  reqVM->getVMQuantity()) {
				shutdown_VM (reqVM->getVM(i));
				reqVM->eraseVM(i);
			}

		} else if (exceededVMs > 0){

			while (exceededVMs != 0){
				shutdown_VM(reqVM->getVM(reqVM->getVMQuantity() -1 ));
				reqVM->eraseVM(reqVM->getVMQuantity() - 1 );
				exceededVMs--;
			}
		}

	} else if ((req->getOperation() == REQUEST_LOCAL_STORAGE) || (req->getOperation() == REQUEST_REMOTE_STORAGE)){

	    // Only execute the pending jobs that the vms of the request have assigned..
        executePendingJobs ();

	} else if (req->getOperation() == REQUEST_FREE_RESOURCES){

		// User has pending jobs | requests?
		if ( (getWQ_size() == 0) && (getRQ_size() == 0) && (checkAllVMShutdown())){
			finalizeUser();
		}
	}
	else {
		showErrorMessage("Unknown request operation in BasicBehavior: %i", req->getOperation());
	}
}

void GeneralUser::requestErrorDeleted (AbstractRequest* req){

	printf ("request deleted: - ");
}

/* --------------------------------------------------------------------
 * ------------------------- Interact with jobs -----------------------
 * --------------------------------------------------------------------
 */

UserJob* GeneralUser::selectJob(){

	// Define ..
		jobBase *job;
		UserJob* jobC;

    // Init ..
		job = nullptr;

	//get the job from the cloudManager

		if (! isEmpty_WQ()){

			job =  getJob_WQ_index(0);

		} else {

			job = nullptr;

		}

		jobC = dynamic_cast<UserJob*> (job);

	return jobC;
}

void GeneralUser::jobHasFinished (jobBase* job){

    UserJob* jobC;
    Machine* m;

    jobC = dynamic_cast<UserJob*>(job);
    m = jobC->getMachine();

    VM* vm = check_and_cast<VM*>(m);

    if (jobC == nullptr) throw cRuntimeError ("GeneralUser::jobHasFinished->job can not be casted to CloudJob\n");

    if ((vm->getNumProcessesRunning() == 1) && (getWQ_size() == 0)) {


        // if the vm is not more needed, request to shutdown it

        shutdown_VM(vm);

    }

}

/* --------------------------------------------------------------------
 * ------------------------- User Behavior ----------------------------
 * --------------------------------------------------------------------
 */

void GeneralUser::schedule(){

	Enter_Method_Silent();

	// Define ..

		UserJob* job;
		jobBase* jobB;
		AbstractRequest* reqB;
		RequestVM* reqVM;
		vector <VM*> setToExecute;
		unsigned int i;
		int quantityVMFree;
		int waitingQSize;
		vector<RequestVM*> requests_vector_storage;
		//vector<RequestVM*>::iterator reqIt;
		bool breakScheduling;
		VM* vm;

	// Init ..

		job = nullptr;
		setToExecute.clear();
		requests_vector_storage.clear();
		quantityVMFree = 0;
		waitingQSize = 0;
		breakScheduling = false;

		// Begin the behavior of the user.
		job = selectJob();

		while ((job != nullptr) && (!breakScheduling)){

			 reqB= selectResourcesJob (job);
			 reqVM = dynamic_cast<RequestVM*>(reqB);

			if (reqVM->getVMQuantity() != 0){

				// Allocate the set of machines where the job is going to execute into the own job

					vm = dynamic_cast<VM*>(reqVM->getVM(0));

					job->setMachine(vm);
					jobB = dynamic_cast<jobBase*>(job);

					if ((vm == nullptr) || (jobB == nullptr)){
					    throw cRuntimeError ("GeneralUser::schedule() -> vm or job == nullptr\n");
					}

					createFSforJob (job, job->getMachine()->getIP(), vm->getNodeSetName(), vm->getNodeName(), vm->getPid());
					delete(reqVM);

			} else {

				// check if the APP needs more VMs than existent free (to allocate smaller jobs)
				for (i = 0; i != machinesMap->size(); i++){
					quantityVMFree += machinesMap->countONMachines(i);
				}

				waitingQSize = getWQ_size();

				if ( (quantityVMFree > 0) && (waitingQSize >= 1) ){

					// There are not enough VMs to execute the first job.
					// It is moved from the first position to the last to execute other job that needs less resources
                    jobB = dynamic_cast<jobBase*>(job);
				    eraseJob_FromWQ(job->getJobId());
				    pushBack_WQ(jobB);

				}

				breakScheduling = true;

			}

			job = selectJob();
		}

		if (job == nullptr){
		    for (i = 0; i < (unsigned int)machinesMap->getMapQuantity(); i++){
                for (int j = 0; j < (int)machinesMap->getSetQuantity(i); j++){

                    Machine* machine = machinesMap->getMachineByIndex(i,j);
                    VM* vm = dynamic_cast<VM*>(machine);

                    if ( (vm->getPendingOperation() == NOT_PENDING_OPS) &&
                         (vm->getState() == MACHINE_STATE_IDLE) &&
                         (vm->getNumProcessesRunning() == 0) ){
                            shutdown_VM(vm);
                    }
                }
		    }

		}
}

} // namespace icancloud
} // namespace inet
