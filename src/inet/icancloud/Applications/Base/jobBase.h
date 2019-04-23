
/**
 * @class jobBase jobBase.h jobBase.cc
 *
 * This class models a job.
 *
 * @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2014-12-12
 */

#ifndef jobBase_H_
#define jobBase_H_

#include "inet/icancloud/Applications/Base/Management/JobResultsSet.h"
#include "inet/icancloud/Applications/Libraries_API/API_OS/API_OS.h"

namespace inet {

namespace icancloud {


class jobBase : virtual public API_OS {

protected:

		/** The Apps module name **/
    	string appType;

        /** Attributes for creating the structures of FS and preload files*/
        vector <fsStructure_T*> fsStructures;
        vector <preload_T*> preloadFiles;

        /** A set of results. The system generates one set per iteration **/
        JobResultsSet* jobResults;

        /* Original paramters */
        int numCopies;
        string originalName;
        int jobState;
        int commId;

protected:

    	/*
    	 * Destructor
    	 */
    	~jobBase();

        /**
         * Module initialization.
         */
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        

        /**
         * Module ending.
         */
         virtual void finish() override;

        /**
         * Process a self message.
         * @param msg Self message.
         */
         virtual void processSelfMessage (cMessage *msg) override = 0;

        /**
         * Process a request message.
         * @param sm Request message.
         */
         virtual void processRequestMessage (Packet *) override = 0;

        /**
         * Process a response message from a module in the local node.
         * @param sm Request message.
         */
         virtual void processResponseMessage (Packet *) override = 0;

public:

        /**
        * Start the app execution.
        */
        virtual void startExecution () override = 0;

        /*
        * Getters and setters for routes at file system
        */
    	void setFSElement(fsStructure_T* elto){fsStructures.push_back(elto);};
    	fsStructure_T* getFSElement(int index){return (*(fsStructures.begin() + index));};
    	int getFSSize(){return fsStructures.size();};
    	vector<fsStructure_T*> getFSComplete(){return fsStructures;};

        /*
        * Getters and setters for routes at file system
        */
    	void setPreloadFile(preload_T* elto){preloadFiles.push_back(elto);};
    	preload_T* getPreloadFile(int index){return (*(preloadFiles.begin() + index));};
    	int getPreloadSize(){return preloadFiles.size();};
    	vector<preload_T*> getPreloadComplete(){return preloadFiles;};
    	void clearPreloadFiles(){preloadFiles.clear();}

    	void setAppType(string name){appType = name;};
    	string getAppType(){return originalName;}; // TODO: Analyze to change the name to apptype

    	void addResults(JobResultsSet* newResult){jobResults = newResult;};
    	JobResultsSet* getResults(){return jobResults;};

        void setJob_startTime(){jobResults->setJob_startTime();};
        void setJob_endTime(){jobResults->setJob_endTime();};

        simtime_t getJob_startTime(){return jobResults->getJob_startTime();};
        simtime_t getJob_endTime(){return jobResults->getJob_endTime();};

        void setOriginalName(string name){originalName = name;};
        string getOriginalName(){return originalName;};
        void setNumCopies(int copies){numCopies = copies;};
        int getNumCopies(){return numCopies;};

        int getCommId(){return commId;};
        void setCommId(int commId_){commId = commId_;};

        int getJobId(){return this->getId();};
        double getTimeToStart();

};

} // namespace icancloud
} // namespace inet

#endif /* JOBBASE */
