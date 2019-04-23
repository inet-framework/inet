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

#include "inet/icancloud/Applications/Base/Management/JobQueue.h"

namespace inet {

namespace icancloud {


JobQueue::JobQueue() {
	idQueue.clear();
}

JobQueue::~JobQueue() {
    idQueue.clear();

    jobBase* job;

    for (int i = 0; i < get_queue_size(); i++){

        job = getJob(i);
        job->callFinish();
        job->deleteModule();
    }
}

void JobQueue::insert_job (jobBase* job, int position){

	int qSize;

	qSize = idQueue.size();

	if (position == -1) idQueue.push_back(job);

	else
	    if ((position <= qSize) && (position >= 0)) idQueue.insert(idQueue.begin()+position, job);

}

jobBase* JobQueue::getJob (int index){

	// Define ..
		jobBase* job;
		vector<jobBase*>::iterator it;
		int qSize;

	// Init..
		job = nullptr;
		it = idQueue.begin();
		qSize = idQueue.size();

	if (index < qSize){

		it += index;
		job = (*it);

	}

	return job;

}

void JobQueue::removeJob (int index){

	//Define ..
		vector<jobBase*>::iterator idQueueIt;

	// Init ..
		idQueueIt = idQueue.begin();

	if (index <= (int)idQueue.size()){

		idQueueIt += index;
		idQueue.erase(idQueueIt);

	}

}

void JobQueue::move_job_from_to (int initPos, int dstPos){

	//Define...
		vector<jobBase*>::iterator it;
		jobBase* job;

	//Init ..

		it = idQueue.begin();

	// Begin ..

		if (initPos != dstPos){

			it += dstPos + 1;

			idQueue.insert(it,job);
			removeJob(initPos);
		}

}

void JobQueue::move_to_qDst  (int position_qSrc, JobQueue* qDst, int position_qDst){

	//Define
	    int qSize;
		jobBase *job;

	//Initialize
		qSize = idQueue.size();
		job = nullptr;

	//Erase from qSrc and insert into qDst;
		job = getJob(position_qSrc);

		if (   ((position_qSrc > -1) && (position_qSrc < qSize) ) ||
		        (position_qDst == -1)){

			qDst -> insert_job(job, position_qDst);
			removeJob(position_qSrc);

		} else {

			printf ("\nError inserting job in position: %d , into destination queue. Maybe the position of job doesn't exist?\n", position_qSrc);

		}
}

void JobQueue::move_from_qSrc (int position_qSrc, JobQueue* qSrc, int position_qDst){

	// Define ..
	    int qSize;
		jobBase *job;

	// Initialize ..
        qSize = idQueue.size();
		job = nullptr;

	// Begin ..;

		job = getJob(position_qSrc);

		if ( (position_qSrc > -1) && (position_qSrc < qSize) ) {

			insert_job(job, position_qDst);
			qSrc->removeJob(position_qSrc);

		} else {

			printf ("\nError inserting job in position: %d , into destination queue. Maybe the position of job doesn't exist?\n", position_qSrc);

		}

}

int JobQueue::get_index_of_job(int jobID){
    // Define ..
        int jobIDAux;
        int i;
        jobBase *job;
        bool found;

    // Initialize ..
        i = 0;
        job = nullptr;
        found = false;

    // Begin ..;

        while ((i < (int)idQueue.size()) && (!found)){
            job = getJob(i);
            jobIDAux = job->getId();

            if (jobIDAux == jobID) {
                found = true;
            }else {
                i++;
            }
        }

        if (!found) i = -1;

        return i;
}

} // namespace icancloud
} // namespace inet
