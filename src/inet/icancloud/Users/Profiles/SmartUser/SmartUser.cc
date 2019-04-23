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

#include "SmartUser.h"

namespace inet {

namespace icancloud {


Define_Module(SmartUser);

SmartUser::~SmartUser() {

	// TODO Auto-generated destructor stub
}

void SmartUser::initialize(int stage){
    AbstractCloudUser::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
		replicasCreated = false;
        printVMs = par("printVMs").boolValue();
        printJobs = par("printJobs").boolValue();
        pendingConnections = 0;
        numApplications = 0;
        freePositionsAtVM.clear();
    }
}

void SmartUser::finish(){
    AbstractCloudUser::finish();
}

void SmartUser::userInitialization(){

	// Init ..

		// Initialize the main parameters of the behavior..
		port = par("port").intValue();
		configMPI = new CfgMPI();

	   // Define ..
            AbstractRequest* vms;

	    // Select the vms to start (all) and then send the request to the cloud manager.
	        vms = selectVMs_ToStartUp ();

        // Start all vms
	        if (vms != nullptr){

                startVMs(vms);
	        }

}

void SmartUser::userFinalization (){

    // Define ..
        int finishQSize,i,j,k;
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

            for (i = 0; i < (int)machinesMap->getMapQuantity(); i++){
                for (j = 0; j < (int)machinesMap->getSetQuantity(i); j++){
                    Machine* machine = machinesMap->getMachineByIndex(i,j);
                    vm = dynamic_cast<VM*>(machine);

                    userVmsData << "#" << vm->getTypeName() << "[" << vm->getNameString() << "]:";

                    for (k = 0; k < vm->getLogStatesQuantity(); k++){
                        vm_state = vm->getLogState(k);

                        if (vm_state->vm_state == MACHINE_STATE_OFF)
                            userVmsData << (vm_state->init_time_M) << ";";
                        userVmsData << (vm_state->vm_state) << ":" ;
                        userVmsData << (vm_state->init_time_M) << ";";
                    }
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


/*
 * --------------------------------------------------------------------
 * ------------------------ Interact with VMs -------------------------
 * --------------------------------------------------------------------
 */
AbstractRequest* SmartUser::selectVMs_ToStartUp (){

    //Define...
        unsigned int i;
        unsigned int setSize;
        bool reqFilled;
        RequestVM* req ;

    //Initialize...
        setSize = vmsToBeSelected.size();
        req = new RequestVM();
        reqFilled = false;

    /*
     *  Obtain all the VMs of the user. Initially all are into VM_NOT_REQUESTED state.
     *  Then, user request for the VMs change state to free, and that is task of the cloud manager.
     *  If user request a VM that is running, the cloud manager discard that request.
     */
    for (i = 0; (i != setSize);i++){
            req->setNewSelection((*(vmsToBeSelected.begin() + i))->type->getType(), (*(vmsToBeSelected.begin() + i))->quantity);
            reqFilled = true;
            pendingVmsAtStartup += (*(vmsToBeSelected.begin() + i))->quantity;
            (*(vmsToBeSelected.begin() + i))->quantity = 0;
    }

    if (!reqFilled) req = nullptr;

    AbstractRequest* reqA;
    reqA = dynamic_cast<AbstractRequest*>(req);

    return reqA;


}

AbstractRequest* SmartUser::selectResourcesJob (jobBase* job){

    // Define ..
        VM* vm;
        Machine* machine;
        vector<VM*> selectedVMs;
        bool found;
        unsigned int i;
        unsigned int j;
        AbstractRequest* aReq;
        RequestVM* reqVM;

    // Init...
        vm = nullptr;
        found = false;
        selectedVMs.clear();
        reqVM = new RequestVM();

    // The behavior of the selection of VMs is to get the first VM in free state.
        for (i = 0; i < machinesMap->size() && (!found); i++){
            for (j = 0; (j < (unsigned int)machinesMap->getSetQuantity(i)) && (!found); j++){

                machine = machinesMap->getMachineByIndex(i,j);
                vm = check_and_cast<VM*>(machine);

                int numCores = (*( ((*(freePositionsAtVM.begin()+ i)).begin()+j) ));

                if (vm->getPendingOperation() == NOT_PENDING_OPS){
                    if (vm->getState() == MACHINE_STATE_IDLE) {
                        if (numCores > 0){
                            selectedVMs.insert(selectedVMs.begin(), vm);
                            found = true;
                            (*( ((*(freePositionsAtVM.begin()+ i)).begin()+j) )) = numCores-1;
                        }
                    }
                }
            }
        }

        reqVM->setVectorVM(selectedVMs);
        aReq = dynamic_cast<AbstractRequest*>(reqVM);

    return aReq;
}

void SmartUser::requestAttended (AbstractRequest* req){

    // Begin ..
        if (req->getOperation() == REQUEST_START_VM){
            RequestVM* reqVM;
            reqVM = dynamic_cast<RequestVM*>(req);

            generateMPIEnv(reqVM);
            pendingVmsAtStartup -= reqVM->getNumberVM();

        } else if ((req->getOperation() == REQUEST_LOCAL_STORAGE) || (req->getOperation() == REQUEST_REMOTE_STORAGE)){

            pendingConnections --;

            if (pendingConnections < 0){
                throw cRuntimeError ("SmartUser::requestAttended -> pendingVmsAtStartup are less than 0...");
            } else if (pendingConnections == 0){
                executePendingJobs ();
            }


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

void SmartUser::requestErrorDeleted (AbstractRequest* req){

	printf ("request deleted: -");
}


/* --------------------------------------------------------------------
 * ------------------------- Interact with jobs -----------------------
 * --------------------------------------------------------------------
 */


UserJob* SmartUser::selectJob(){

    // Init ..
        jobBase *job;
        UserJob* jobC;
        vector<VM*> vms;
        UserJob* job_copy;
        jobBase* job_copyB;
        ostringstream jobID;

    // Define ..
        job = nullptr;

    //get the job from the cloudManager
        if (! isEmpty_WQ()){

            job =  getJob_WQ_index(0);

            if (!replicasCreated){
            // Calculates the number of apps.
                if (getWQ_size() != 1){
                   throw cRuntimeError("The waitingQ for SmartUser has to be only one..");
                }else{
                    job = getJob_WQ_index(0);
                }

                for (int i = 0; i < numApplications; ){
                        jobC = check_and_cast<UserJob*> (job);
                        job_copy = jobC->cloneJob(this);

                        //Add the parameter rank to each copy of the job
                            job_copy->par("myRank").setIntValue(i);

                        // A job per vm (per core). Insert into running queue with different id
                            jobID.clear();
                            jobID.str(std::string());
                            jobID <<  job->getAppType() << "_process_" << i;

                            job_copy->setAppType(jobID.str().c_str());

                            job_copyB = check_and_cast<UserJob*> (job_copy);
                            pushBack_WQ(job_copyB);

                        i++;
                }

                // Delete job
                eraseJob_FromWQ(job->getJobId());

                // And one all replicas are created, we have to select the first one
                job = getJob_WQ_index(0);

                replicasCreated = true;
            }

        } else {

            job = nullptr;

        }
        jobC = dynamic_cast<UserJob*> (job);

    return jobC;
}


void SmartUser::jobHasFinished (jobBase* job){

    UserJob* jobC;
    VM* vm;
    Machine* m;

    jobC = dynamic_cast<UserJob*>(job);
    m = jobC->getMachine();
    vm = check_and_cast<VM*>(m);

    if (jobC == nullptr) throw cRuntimeError ("GeneralUser::jobHasFinished->job can not be casted to CloudJob\n");

    if ((vm->getNumProcessesRunning() == 1) && (getWQ_size() == 0)) {


        // if the vm is not more needed, request to shutdown it

        shutdown_VM(vm);

    }
}


/* ---------------------------------------------------------------------------------------------------
 * ------------------------- give the values of the processes mpi to each app  -----------------------
 * ---------------------------------------------------------------------------------------------------
 */



void SmartUser::generateMPIEnv(RequestVM *req){

	int i, j;
	int size;
	int numCPUs;
	int lastRank;
	VM* vm;

	size = req->getNumberVM();

	if (configMPI->processVector.empty()){
		lastRank = 0;
	}

	for (i = 0; i < size; i++){
		vm = req->getVM(i);
		numCPUs = vm->getNumCores();

		for (j = 0; j < numCPUs; j++){
			configMPI -> insertMPI_node_config(vm->getIP(),port+lastRank,lastRank);
			lastRank++;
		}

	}

}

void SmartUser::generateNumberOfAppReplicas (){

    int numCPUs = 0;
    unsigned int i, j;
    VM* vm;
    Machine* machine;
    vector<int> numCoresVector;
    numCoresVector.clear();

    // The behavior of the selection of VMs is to get the first VM in free state.
        for (i = 0; i < machinesMap->size(); i++){
            for (j = 0; (int)j < machinesMap->getSetQuantity(i); j++){

                machine = machinesMap->getMachineByIndex(i,j);
                vm = check_and_cast<VM*>(machine);

                numCPUs = vm->getNumCores();
                numApplications += numCPUs;

                numCoresVector.push_back(numCPUs);
            }

            freePositionsAtVM.push_back(numCoresVector);
            numCoresVector.clear();

        }


}

/* --------------------------------------------------------------------
 * ------------------------- User Behavior ----------------------------
 * --------------------------------------------------------------------
 */

void SmartUser::schedule(){

	Enter_Method_Silent();

	// Define ..

        UserJob* job;
        vector <VM*> setToExecute;
        int numCPUs;
        vector<RequestVM*> requests_vector_storage;
        //vector<RequestVM*>::iterator reqIt;
        AbstractRequest* reqB;
        RequestVM* reqVM;
        VM* vm;

    // Init ..

        job = nullptr;
        setToExecute.clear();
        requests_vector_storage.clear();
        numCPUs = 0;

		// Begin the behavior of the user...
		if (pendingVmsAtStartup == 0){

	        // Calculate the number of app replicas..
	            if (!replicasCreated) {
	                generateNumberOfAppReplicas();
	                pendingConnections = numApplications;
	            }

		    job = selectJob();

		    while(job != nullptr){

		        reqB = selectResourcesJob (job);
		        reqVM = dynamic_cast<RequestVM*>(reqB);

                if (reqVM->getVMQuantity() == 0)
                    throw cRuntimeError ("Error at user scheduling mpi: setToExecute size = 0 .. ");

                while (reqVM->getVMQuantity() != 0){

                    numCPUs = reqVM->getVM(0)->getNumCores();

                    while (numCPUs != 0){
                            vm = dynamic_cast<VM*>(reqVM->getVM(0));
                            job->setMachine(vm);
                            createFSforJob (job, vm->getIP(), vm->getNodeSetName(), vm->getNodeName(), vm->getPid());

                            job = selectJob();
                            numCPUs--;

                            int waitingS = getWQ_size();
                            if ((waitingS == 0) && (numCPUs > 0)) throw cRuntimeError ("The number of processes does not match with the number of processors!..");
                    }

                    reqVM->eraseVM(0);
                }
                delete(reqB);
		    }

		    if (!isEmpty_WQ()) throw cRuntimeError ("Has been created more jobs than vms cpus available.");
		}
}


} // namespace icancloud
} // namespace inet
