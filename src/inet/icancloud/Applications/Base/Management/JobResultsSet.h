/**
 * @class JobResultsSet JobResultsSet.cc JobResultsSet.h
 *
 * This class allocates all the results of an application
 *
 * @authors Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2011-15-08
 */

#ifndef JOBRESULTSSET_H_
#define JOBRESULTSSET_H_

#include "inet/icancloud/Base/icancloud_Base.h"
#include "inet/icancloud/Applications/Base/Management/JobResults.h"

namespace inet {

namespace icancloud {


class JobResultsSet {

private:

	/** vector that allocate the results of one execution of one job **/
	vector <JobResults*> jobResultsSet;

    /** The identifier of the job. The userName and the name of the application **/
	string jobID;

    /** The application marks of time.  **/
    simtime_t job_startTime;
    simtime_t job_endTime;

    /** The type of the machine where the job has been executed */
    string mType;

public:

	/*
	 * Constructor.
	 */
	JobResultsSet();

    /*
     * Destructor.
     */
    virtual ~JobResultsSet();

    /*
     * finalization
     */
    void finishJobResultsSet();
    /*
     * This method returns the size of the vector.
     */
    int getJobResultSize ();

    /*
	 * This method returns the results set allocated in the vector jobResultsSet
	 * @param: the index of the jobResults in the vector or the id name of the results.
	 */
	JobResults* getJobResultSet (int index) {return (*(jobResultsSet.begin() + index));};
	JobResults* getJobResultSet (string value);

    /*
     * Creates the new job result set to allocate before the job results
     */
	void newJobResultSet(string jobResult);

	/*
     * These methods allocate jobResults in a vector at the position given by index.
     */
    void setJobResult (string jobResultsName, int value);
    void setJobResult (string jobResultsName, double value);
    void setJobResult (string jobResultsName, simtime_t value);
    void setJobResult (string jobResultsName, bool value);
    void setJobResult (string jobResultsName, string value);

    void setJobResult (int indexJobResult, int value);
    void setJobResult (int indexJobResult, double value);
    void setJobResult (int indexJobResult, simtime_t value);
    void setJobResult (int indexJobResult, bool value);
    void setJobResult (int indexJobResult, string value);

    /*
     * Setter and getter for the identificator of the job
     */
    void setJobID(string name){jobID = name;};
    string getJobID(){return jobID;};

    /*
     * Setters for the start time when application begins to execute
     */
    void setJob_startTime(simtime_t simtime);
    void setJob_startTime(){job_startTime = simTime();};

    /*
     * Setters for the end time when application finalizes its execution
     */
    void setJob_endTime(simtime_t simtime);
    void setJob_endTime(){job_endTime = simTime();};

    /*
     * Getters for start and end times
     */
    simtime_t getJob_startTime(){return job_startTime;};
    simtime_t getJob_endTime(){return job_endTime;};

    /*
     * This methods returns the machine where application is executed
     */
    void setMachineType(string name){mType = name;};
    string getMachineType(){return mType;};

    /*
     * This method duplicates the results of a job execution
     */
    JobResultsSet* dup ();

	/*
	 * Method that return a string with the result set to print it.
	 */
	string toString();

private:
	/*
     * This method allocate the jobResults in the vector at the position given by index.
     * @param: the jobResults pointer.
     * @param: the index of the position in the vector
     */
    void setJobResult_p (string jobResultsName, string value);
    void setJobResult_p (int indexJobResult, string value);
};

} // namespace icancloud
} // namespace inet

#endif /* JOBRESULTSSET_H_ */
