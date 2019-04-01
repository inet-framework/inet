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

#include "queuesManager.h"

namespace inet {

namespace icancloud {


queuesManager::~queuesManager() {
}

void queuesManager::finish(){
    icancloud_Base::finish();

    waitingQueue->clear();
    runningQueue->clear();
    finishQueue->clear();

    jobResults.clear();
}

void queuesManager::initialize(int stage) {

    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // Initialize structures and parameters
        waitingQueue = new JobQueue();
        waitingQueue->clear();

        runningQueue = new JobQueue();
        runningQueue->clear();

        finishQueue = new JobQueue();
        finishQueue->clear();
    }

}


void queuesManager::addParsedJob (jobBase *job){waitingQueue->insert_job(job);};


int queuesManager::getWQ_size(){return waitingQueue->size();};

int queuesManager::getRQ_size(){return runningQueue->size();};

int queuesManager::getFQ_size(){return finishQueue->size();};

bool queuesManager::isEmpty_WQ(){return waitingQueue->isEmpty();};

bool queuesManager::isEmpty_RQ(){return runningQueue->isEmpty();};

bool queuesManager::isEmpty_FQ(){return finishQueue->isEmpty();};

bool queuesManager::eraseJob_FromWQ (int jobID){

	//Define...

	    unsigned int i;
		//vector <jobBase*>::iterator jobIt;
		bool found;

	//Initialize...

		i = 0;
		found = false;

	//Search the job into jobsList
		for (i = 0; (waitingQueue->get_queue_size()) && (!found); i++ ){

			if (waitingQueue->getJob(i)->getId() == jobID){

				waitingQueue->removeJob(i);
				found = true;
			}
		}
		return found;
}

bool queuesManager::eraseJob_FromRQ (int jobID){

	//Define...

	    unsigned int i;
		//vector <jobBase*>::iterator jobIt;
		bool found;

	//Initialize...

		i = 0;
		found = false;

	//Search the job into jobsList
		for (i = 0; (runningQueue->get_queue_size()) && (!found); i++ ){

			if (runningQueue->getJob(i)->getId() == jobID){

				runningQueue->removeJob(i);
				found = true;
			}
		}
		return found;
}

bool queuesManager::eraseJob_FromFQ (int jobID){

	//Define...

	    unsigned int i;
		//vector <jobBase*>::iterator jobIt;
		bool found;
	//Initialize...

		i = 0;
		found = false;
	//Search the job into jobsList
		for (i = 0; (finishQueue->get_queue_size()) && (!found); i++ ){

			if (finishQueue->getJob(i)->getId() == jobID){

				finishQueue->removeJob(i);
				break;
				found = true;
			}
		}

		return found;
}

void queuesManager::pushBack_WQ(jobBase* job){
    (waitingQueue->insert_job(job));
}

void queuesManager::pushBack_RQ(jobBase* job){runningQueue->insert_job(job);};

void queuesManager::pushBack_FQ(jobBase* job){finishQueue->insert_job(job);};

void queuesManager::insert_WQ(jobBase* job, int position){waitingQueue->insert_job(job,position);};

void queuesManager::insert_RQ(jobBase* job, int position){runningQueue->insert_job(job,position);};

void queuesManager::insert_FQ(jobBase* job, int position){finishQueue->insert_job(job,position);};

void queuesManager::move_WQ (int positionInitial, int positionFinal){waitingQueue->move_job_from_to(positionInitial, positionFinal);};

void queuesManager::move_RQ (int positionInitial, int positionFinal){runningQueue->move_job_from_to(positionInitial, positionFinal);};

void queuesManager::move_FQ (int positionInitial, int positionFinal){finishQueue->move_job_from_to(positionInitial, positionFinal);};

jobBase* queuesManager::getJob_WQ (int jobID){return get_job_by_ID(jobID, waitingQueue);};

jobBase* queuesManager::getJob_RQ (int jobID){return get_job_by_ID(jobID, runningQueue);};

jobBase* queuesManager::getJob_FQ (int jobID){return get_job_by_ID(jobID, finishQueue);};

jobBase* queuesManager::getJob_WQ_index  (int index){return get_job_by_index (index, waitingQueue);};

jobBase* queuesManager::getJob_RQ_index  (int index){return get_job_by_index (index, runningQueue);};

jobBase* queuesManager::getJob_FQ_index  (int index){return get_job_by_index (index, finishQueue);};

jobBase* queuesManager::getJobByModID_WQ (int modID){return get_job_by_ModID (modID, waitingQueue);};

jobBase* queuesManager::getJobByModID_RQ (int modID){return get_job_by_ModID (modID, runningQueue);};

jobBase* queuesManager::getJobByModID_FQ (int modID){return get_job_by_ModID (modID, finishQueue);};

int queuesManager::getIndex_WQ (int jobID){return getIndexOfJob(jobID, waitingQueue);};

int queuesManager::getIndex_RQ (int jobID){return getIndexOfJob (jobID, runningQueue);};

int queuesManager::getIndex_FQ (int jobID){return getIndexOfJob (jobID, runningQueue);};

void queuesManager::moveFromWQ_toRQ (int jobID){
    //Define
        jobBase* job;
        bool found;
        int index;
    //Initialize
        found = false;
        index = 0;
    //Erase from qSrc and insert into qDst;

         while ((index < waitingQueue->size()) && (!found)){

             job = waitingQueue->getJob(index);
                if (job->getId() == jobID){

                    found = true;
                    index++;
                }
            }

    waitingQueue->move_to_qDst(index ,runningQueue, runningQueue->get_queue_size());
}

void queuesManager::moveFromRQ_toFQ (int jobID){
    //Define
        jobBase* job;
        bool found;
        int index;
    //Initialize
        found = false;
        index = 0;
    //Erase from qSrc and insert into qDst;

         while ((index < runningQueue->size()) && (!found)){
             job = runningQueue->getJob(index);
             if (job->getId() == jobID){

                    found = true;
             }else{
                    index++;
             }
         }

    runningQueue->move_to_qDst(index ,finishQueue, finishQueue->get_queue_size());
}


void queuesManager::moveFromWQ_toFQ (int jobID){
    //Define
        jobBase* job;
        bool found;
        int index;
    //Initialize
        found = false;
        index = 0;
    //Erase from qSrc and insert into qDst;

         while ((index < waitingQueue->size()) && (!found)){

             job = waitingQueue->getJob(index);

                if (job->getId() == jobID){
                    found = true;
                }else{
                    index++;
                }
            }

    waitingQueue->move_to_qDst(index ,finishQueue, finishQueue->get_queue_size());
}

jobBase* queuesManager::get_job_by_ID (int jobID, JobQueue* jq){

	//Define...

	    unsigned int i;
		jobBase * job;
		//vector <jobBase*>::iterator jobIt;
		bool found;

	//Initialize...
		found = false;
		i = 0;

	//Search the job into jobsList
		for (i = 0; (jq->get_queue_size()) && !(found); i++ ){

			if (jq->getJob(i)->getId() == jobID){

				job = jq->getJob(i);
				found = true;
			}
		}

		return job;

}

jobBase* queuesManager::get_job_by_index (int index, JobQueue* jq){

	// Define ...

		jobBase * job;

	// Initialize...

		job = nullptr;

	// Begin ..

		if (index <= jq->get_queue_size()-1){

			job = jq->getJob(index);

		}

		return job;

}

jobBase* queuesManager::get_job_by_ModID (int modID, JobQueue* jq){

    // Define ...
        jobBase * job;
        bool found;
        int index;

    // Initialize...
        job = nullptr;
        found = false;
        index = 0;

    // Begin ..

        while ((index < jq->get_queue_size() ) && (!found)){

            job = jq->getJob(index);

            if (job->getId() == modID){
                found = true;
            }

            index++;

        }

        return job;
}

int queuesManager::getIndexOfJob (int jobID, JobQueue *jq){

	//Define...
	    int i;
	    int index;
	    bool found;

	//Initialize...
		i = 0;
		index = -1;
		found = false;

	//Search the job into jobsList

		for (i = 0; (jq->get_queue_size()) && !(found); i++ ){

			if (jq->getJob(i)->getId() == jobID){

				index = i;
				found = true;
			}
		}

		return index;
}



} // namespace icancloud
} // namespace inet
