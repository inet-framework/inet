/**
 * @class queuesManager queuesManager.cc queuesManager.h
 *
 * This class allocates all the jobs at a queue depending on its state.
 * If a job is waiting for its execution, it is allocated at waiting queue.
 * If it is performing operations it is allocated at running queue.
 * When a job has finished, it is moved from running queue to finish queue.
 *
 * This class supply all the methods to manage jobs queues of a user.
 *
 * @authors Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2012-06-10
 */

#ifndef _QMANAGER_H_
#define _QMANAGER_H_

#include "inet/icancloud/Applications/Base/Management/JobResults.h"
#include "inet/icancloud/Applications/Base/Management/JobQueue.h"
#include "inet/icancloud/Applications/Base/Management/JobResultsSet.h"

#include "inet/icancloud/Base/icancloud_Base.h"

namespace inet {

namespace icancloud {


class queuesManager :  public icancloud_Base{

protected:


        /** Structures to jobs management*/
        JobQueue* waitingQueue;
        JobQueue* runningQueue;
        JobQueue* finishQueue;

        /** Structure for results */
        vector<JobResultsSet*> jobResults;

        /* User finalization.. */
        bool userFinalizing;

        /*
        * Destructor.
        */
        virtual ~queuesManager ();

        /**
        * Module ending.
        */
        void finish() override;

        /**
        * Module initialization.
        */
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }

        /*
        * This method add a job to jobList when it is parsed from the parameters defined in omnetpp.ini
        */
        void addParsedJob (jobBase *job);


        /*
        *  Returns number of jobs of the waiting queue
        */
        int getWQ_size();

        /*
        *  Returns number of jobs of the running queue
        */
        int getRQ_size();

        /*
        *  Returns number of jobs of the finish queue
        */
        int getFQ_size();

        /*
        *  Check if there are no jobs in the waiting queue. The method returns true if the queue is empty
        *  and false otherwise.
        */
        bool isEmpty_WQ();

        /*
        *  Check if there are no jobs in the running queue. The method returns true if the queue is empty
        *  and false otherwise.
        */
        bool isEmpty_RQ();

        /*
        *  Check if there are no jobs in the finish queue. The method returns true if the queue is empty
        *  and false otherwise.
        */
        bool isEmpty_FQ();

        /*
        * Erase job from waiting queue by jobID
        */
        bool eraseJob_FromWQ (int jobID);

        /*
        * Erase job from running queue by jobID
        */
        bool eraseJob_FromRQ (int jobID);

        /*
        * Erase job from finish queue by jobID
        */
        bool eraseJob_FromFQ (int jobID);

        /*
        * insert job at the last position of the waiting queue
        */
        void pushBack_WQ (jobBase* job);

        /*
        * insert job at the last position of the running queue
        */
        void pushBack_RQ(jobBase* job);

        /*
        * insert job at the last position of the finish queue
        */
        void pushBack_FQ(jobBase* job);

        /*
        * Insert job into the waiting queue at the given position as parameter
        */
        void insert_WQ(jobBase* job, int position);

        /*
        * Insert job into the running queue at the given position as parameter
        */
        void insert_RQ(jobBase* job, int position);

        /*
        * Insert job into the finish queue at the given position as parameter
        */
        void insert_FQ(jobBase* job, int position);

        /*
        * move a job from a positionInitial to positionFinal in the waiting queue
        */
        void move_WQ (int positionInitial, int positionFinal);

        /*
        * move a job from a positionInitial to positionFinal in the running queue
        */
        void move_RQ (int positionInitial, int positionFinal);

        /*
        * move a job from a positionInitial to positionFinal in the finish queue
        */
        void move_FQ (int positionInitial, int positionFinal);

        /*
        * This methods returns the job with the same id as the parameter jobID in the waiting queue.
        */
        jobBase* getJob_WQ (int jobID);

        /*
        * This methods returns the job with the same id as the parameter jobID in the running queue.
        */
        jobBase* getJob_RQ (int jobID);

        /*
        * This methods returns the job with the same id as the parameter jobID in the finish queue.
        */
        jobBase* getJob_FQ (int jobID);

        /*
        * This methods returns the job situated in the waiting queue at the position given by index parameter.
        */
        jobBase* getJob_WQ_index (int index);

        /*
        * This methods returns the job situated in the running queue at the position given by index parameter.
        */
        jobBase* getJob_RQ_index (int index);

        /*
        * This methods returns the job situated in the finish queue at the position given by index parameter.
        */
        jobBase* getJob_FQ_index (int index);

        /*
        * This methods returns the job situated in the waiting queue at the position given by index parameter.
        */
        jobBase* getJobByModID_WQ (int modID);

        /*
        * This methods returns the job situated in the running queue at the position given by index parameter.
        */
        jobBase* getJobByModID_RQ (int modID);

        /*
        * This methods returns the job situated in the finish queue at the position given by index parameter.
        */
        jobBase* getJobByModID_FQ (int modID);

        /*
        * This method returns the index of a job with the same id as given parameter (jobID) in the waiting queue
        */
        int getIndex_WQ (int jobID);

        /*
        * This method returns the index of a job with the same id as given parameter (jobID) in the running queue
        */
        int getIndex_RQ (int jobID);

        /*
        * This method returns the index of a job with the same id as given parameter (jobID) in the finish queue
        */
        int getIndex_FQ (int jobID);

        /*
        * This method moves the job with the id = jobID, from the waiting queue to the running queue
        */
        void moveFromWQ_toRQ (int jobID);

        /*
        * This method moves the job with the id = jobID, from the running queue to the finish queue
        */
        void moveFromRQ_toFQ (int jobID);

        /*
        * This method moves the job with the id = jobID, from the running queue to the finish queue
        */
        void moveFromWQ_toFQ (int jobID);



        // ---------------------------------------------------------------------------
        // ---------------------- OPERATION WITH JOB RESULTS  --------------------
        // ---------------------------------------------------------------------------

        /*
        * This method link a job result set to a concrete job
        */
        void setJobResults(JobResultsSet* set){jobResults.push_back(set);};

        /*
        * This method is a getter of job results. It returns the job results allocated at results structure
        */
        JobResultsSet* getJobResults(int index){return (*(jobResults.begin()+index));};

        /*/
        *   This method returns the quantity of results allocated at job structure.
        */
        int getJobResultsSize(){return jobResults.size();};


private:

        /*
        * This methods returns a job in the vector waitingQueue with the ID == jobID;
        * @Param: The ID of the job.
        */
        jobBase* get_job_by_ID (int jobID, JobQueue* jq);

        /*
        * This methods returns a user in the vector waitingQueue at the "index" position.
        * @Param: The index of the job in the waiting queue.
        */
        jobBase* get_job_by_index (int index, JobQueue* jq);

        /*
        * This methods returns a user in the vector waitingQueue at the "index" position.
        * @Param: The index of the job in the waiting queue.
        */
        jobBase* get_job_by_ModID (int modID, JobQueue* jq);


        /*
        * This method returns the index of a job in a given queue.
        * @Param: The ID of the job.
        * @Param: A pointer to the queue where the method is going to search
        */
        int getIndexOfJob (int jobID, JobQueue *qSrc);
};

} // namespace icancloud
} // namespace inet

#endif
