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

#include "inet/icancloud/Users/AbstractUser.h"

namespace inet {

namespace icancloud {


AbstractUser::~AbstractUser() {
}

void AbstractUser::initialize(int stage) {

    userStorage::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {

        // Initialize structures and parameters
        waitingQueue = new JobQueue();
        waitingQueue->clear();

        waiting_for_system_response = new JobQueue();
        waiting_for_system_response->clear();

        runningQueue = new JobQueue();
        runningQueue->clear();

        finishQueue = new JobQueue();
        finishQueue->clear();

        userFinalizing = false;

        startTime = simTime();
    }
}

void AbstractUser::finish(){
    userStorage::finish();
}

bool AbstractUser::finalizeUser (){

    // Define ..
        jobBase* job;

    // Init ..
        job = nullptr;

    // Begin ..

        if (!hasPendingRequests()){

            userFinalizing = true;
            endTime = simTime();

            // Supress all the Jobs from the Waiting Queue to the Finish Queue.
            while (!(waitingQueue->isEmpty())){

                job = waitingQueue->getJob(0);

                job -> setJob_startTime ();
                job -> setJob_endTime ();

                waitingQueue->move_to_qDst(0,finishQueue,finishQueue->get_queue_size());
            }


            userFinalization();

            while (finishQueue->isEmpty())
                finishQueue->removeJob(0);

           abandonSystem();

        } else {
            userFinalizing = false;
        }

        return userFinalizing;
}

string AbstractUser::printJobResults (JobResultsSet* resultSet){

    // Define ...
        ostringstream resString;
        JobResults* result;
        string jobName;
        jobName =resultSet->getJobID().c_str();
      if (!jobName.empty())
          resString << "JobName;" << resultSet->getJobID().c_str() << "\n";

      resString << "Beginning_Time;" << resultSet->getJob_startTime ();
      resString << ";TimeToExecute;" << resultSet->getJob_endTime() - resultSet->getJob_startTime () << "\n";


      for (int i = 0; i < resultSet->getJobResultSize(); i++){

          // Print the result of the iteration
          result = resultSet-> getJobResultSet(i);

          resString << result ->getName().c_str();
          for (int j = 0; j < result->getValuesSize(); j++)
              resString << ";" << result ->getValue(j) << ";";

      }
      resString << "\n";

      return resString.str().c_str();
}

void AbstractUser::initParameters(string userBehavior, string userId, string fileLog) {

    userID = this->getId();
    userName = userId;

    if ((strcmp(fileLog.c_str(), "") != 0)) {
        print = true;
        file = fileLog;
    }
    else {
        print = false;
    }

}

void AbstractUser::addParsedJob (jobBase *job){waitingQueue->insert_job(job);};

void AbstractUser::start_up_job_execution (AbstractNode* destinationExecute, jobBase* job, JobQueue* qSrc, JobQueue* qDst, int qDst_pos){
    printf("TODO AbstractUser::start_up_job_execution\n");
        endSimulation();
//    // Define ...
//        string jobId;
//        VM* vm;
//        vector<VM*>::iterator setVMit;
//        jobBase* jobAux;
//        int index = 0;
//        bool found = false;
//    // Initialize...
//
//        jobId = job->getJobID();
//
//    //move from the qSrc to the scheduler queue of running jobs
//
//        if (qSrc == nullptr){
//
//            while ((index < waitingQueue->size()) && (!found)){
//
//                jobAux = waitingQueue->getJob(index);
//
//             if ((strcmp (jobAux->getJobID().c_str(),jobId.c_str())) == 0){
//
//                    found = true;
//                    index++;
//                }
//            }
//
//            if (qDst == nullptr){
//                waitingQueue->move_to_qDst(index, runningQueue, qDst_pos);
//            }
//            else {
//                waitingQueue->move_to_qDst(index, qDst, qDst_pos);
//            }
//
//        }else{
//
//
//            while ((index < qSrc->size()) && (!found)){
//
//                jobAux = qSrc->getJob(index);
//
//                if ((strcmp (jobAux->getJobID().c_str(),jobId.c_str())) == 0)
//                    found = true;
//                else
//                    index++;
//            }
//
//            if (qDst == nullptr){
//                qSrc->move_to_qDst (index, runningQueue, qDst_pos);
//            }
//            else {
//                qSrc->move_to_qDst (index, qDst, qDst_pos);
//            }
//
//        }
//
//        for (int i = 0; i < job->getVMToExecute_size(); i++){
//
//            vm = job->getVMToExecute(i);
//            allocateJob(job, vm);
//
//            if (vm->getVmState() != VM_STATE_APP_ERROR){
//
//                    // Record the job data
//                        job->setJob_startTime();
//                        job->setVM(vm);
//
//                    // Start scope and app
//                        vm->setVmState(VM_STATE_RUNNING);
//                        job->startExecution ();
//            }
//
//        }
}

void AbstractUser::notify_UserJobHasFinished (jobBase* job){

    string jobID;

    // Begin ..

        /* Record the instant of job's finalization */
        job->setJob_endTime();

        setJobResults(job->getResults()->dup());

        /* User virtual method */
        jobHasFinished(job);

        //Finalize the job and move it to the finish queue
            moveFromRQ_toFQ(job->getId());

        // free the resources
//           deleteJobVM(job->getVM(), job);

	if (getWQ_size() != 0) {

//		executePendingJobs();

		schedule();

	}

}

void AbstractUser::notify_UserRequestAttendeed  (AbstractRequest* req){

    throw cRuntimeError ("void AbstractUser::notify_UserRequestAttendeed ->to be implemented\n");
    // Define ..
	//	vector<AbstractRequest*>::iterator reqIt;
//		int i;
//		int storageConnectionSize;
//		StorageRequest* strReq;

	// Initialize ..
//		i = 0;

	/*
	 * -------------------------------------------------------------
	 * --------------- PRE-CALL -> requestAttended -----------------
	 * -------------------------------------------------------------
	 */


	if (req->getOperation() == REQUEST_RESOURCES){
		requestAttended (req);

		// This method (userBase) delete the request from the pending_requests vector.
		requestArrival(req);
	}

//	DE AQUI EN ADELANTE, NO REVISADO!!
	else if (req->getOperation() == REQUEST_FREE_RESOURCES){
		requestAttended (req);
	}
	else if (req->getOperation() == REQUEST_REMOTE_STORAGE){

//	    strReq = check_and_cast<StorageRequest>(req);
//  	    storageConnectionSize = strReq->getConnectionSize();
//
//		for (i = 0; i < storageConnectionSize; i++){
//			vm = req->getStorageResponseVM(i);
//
//			// Delete from the control structure
//			vm_erased = eraseVMFromControlVector(vm, USER_REMOTE_STORAGE);
//			if (!vm_erased) showErrorMessage("Trying to erase a vm from vms_waiting_for_remote_storage vector which not exists (User:%s, vm:%s[%i]",vm->getUserID().c_str(), vm->getType().c_str(), vm->getVmID());
//
//			// Get the module and initialize it!
//			if (!vmHasStorageRequests(vm)){
//				vm->setVmState(VM_STATE_FREE);
//		        vm->setPendingOperation(VM_NOT_PENDING_OPS);
//
//		        // notify to the user the vm loaded for executing jobs..
//		            req_dup = new RequestBase();
//                    reqBase->setOperation(USER_REMOTE_STORAGE);
//		            vmToNotify.push_back(vm);
//		            reqBase->setVectorVM(vmToNotify);
//
//		            requestAttended (reqBase);
//
//		            delete(reqBase);
//			}
//		}

	}else if (req->getOperation() == REQUEST_LOCAL_STORAGE){

//        strReq = check_and_cast<StorageRequest>(req);
//		storageConnectionSize = req->getConnectionSize();
//
//		for (i = 0; i < storageConnectionSize; i++){
//			vm = strReq->getStorageResponseVM(i);
//
//			// Delete from the control structure
//			vm_erased = eraseVMFromControlVector(vm, USER_LOCAL_STORAGE);
//			if (!vm_erased) showErrorMessage("Trying to erase a vm from vms_waiting_for_remote_storage vector which not exists (User:%s, vm:%s[%i]",vm->getUserID().c_str(), vm->getType().c_str(), vm->getVmID());
//
//			if (!vmHasStorageRequests(vm)){
//				vm->setVmState(VM_STATE_FREE);
//		        vm->setPendingOperation(VM_NOT_PENDING_OPS);
//			}
//
//            // notify to the user the vm loaded for executing jobs..
//                req_dup = new RequestBase();
//                reqBase->setOperation(USER_REMOTE_STORAGE);
//                vmToNotify.push_back(vm);
//                reqBase->setVectorVM(vmToNotify);
//
//                requestAttended (reqBase);
//
//                delete(reqBase);
//
//		}
	}

	/*
	 * -------------------------------------------------------------
	 * --------------- POST-CALL -> requestAttended ----------------
	 * -------------------------------------------------------------
	 */
		// Begin ..
//		if ((req->getOperation() == USER_START_VMS) && (!isEmpty_WQ())){
//
//			// Select the vms for request remote storage!
//				while (req->getNumberVM() != 0){
//
//					// get the first vm form the request
//						vm = req->getVM(0);
//
//					// Delete from the control structure
//						vm_erased = eraseVMFromControlVector(vm, USER_REQUEST_START_VMS);
//						if (!vm_erased) showErrorMessage("DEBUG_CLOUD_USE->Trying to erase a vm from vms_to_initialize vector which not exists (User:%s, vm:%s[%i]",vm->getUserID().c_str(), vm->getType().c_str(), vm->getVmID());
//
//					// Start the vms
//                        vm->setVmState(VM_STATE_FREE);
//                        vm->setPendingOperation(VM_NOT_PENDING_OPS);
//						vm->getVmmodule()->callInitialize();
//
//					// Delete from the request
//						req->eraseVM(0);
//
//				}
//		}
//
//

	if (!userFinalizing)  schedule();
}



} // namespace icancloud
} // namespace inet
