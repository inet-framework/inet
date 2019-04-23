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

#ifndef JOBQUEUE_H_
#define JOBQUEUE_H_

#include "inet/icancloud/Applications/Base/jobBase.h"

#include <omnetpp.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdexcept>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"

namespace inet {

namespace icancloud {


using std::vector;
using std::string;

class JobQueue {

protected:

    vector <jobBase*> idQueue;

  public:

    /*
     * Constructor
     */
	JobQueue();

	/*
	 * Destructor
	 */
	virtual ~JobQueue();

	/*
	 * This method returns the size of the given queue
	 * @Param: The queue where is going to obtain the size.
	 */
	int get_queue_size(){return idQueue.size();};

    /*
     * This method insert into the queue a job in the position given.
     * If the position is mayor than the size of the vector, the method will return -1
     */
	void insert_job (jobBase* job, int position = -1);

	/*
	 * These methods returns the Cloud Job into the queue. If there is several instances of cloud job with the same JobID, the method
	 * returns the first position.
	 */
	jobBase* getJob (int index);

	/*
	 * These methods erase the job from the vector and return the jobBase to insert it in another queue or to a total deletion
	 */
	void removeJob (int index);

	/*
	 * This method move a job from a position at the waiting queue to another position
	 * @Param: The initial position
	 * @Param: The final position
	 */
	void move_job_from_to (int initPos, int dstPos);

	/*
	 * This method move a job from a vector source to a destination source.
	 */
	void move_to_qDst (int position_qSrc, JobQueue* qDst, int position_qDst = -1);

    /*
     * This method move a job from a vector source to a destination source.
     */
	void move_from_qSrc (int position_qSrc, JobQueue* qSrc, int position_qDst = -1);

	/*
	 * This method returns the quantity of jobs allocated at queue
	 */
	int size(){return idQueue.size();};

	/*
	 * This method erase all the jobs allocated at queue
	 */
	void clear (){idQueue.clear();};

	/*
	 * This method return true if queue size is major than 1
	 */
	bool isEmpty (){return (idQueue.size() == 0);};

	int get_index_of_job(int jobID);

};

} // namespace icancloud
} // namespace inet

#endif /* JOBQUEUE_H_ */
