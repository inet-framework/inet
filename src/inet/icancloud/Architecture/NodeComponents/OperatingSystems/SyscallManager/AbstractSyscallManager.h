 /*
  * This abstract class is the base to implement a class for a message redirector class
  *  between applications and hardware resources.
  *  This class is responsible for operating system functionalities. It controls the amount of memory, storage of the
  *  base system, the quantity of applications linked to the machine, and linking and unlinking tenant processes to the system.
  * @author - updated to iCanCloud by Gabriel González Castañé
  * @date 2012-05-17
  */

#ifndef __ABSTRACTSYSCALLMANAGER_H_
#define __ABSTRACTSYSCALLMANAGER_H_

#include "inet/icancloud/Base/icancloud_Base.h"

namespace inet {

namespace icancloud {


class AbstractSyscallManager : virtual public icancloud_Base{

protected:

    struct processRunning_t{
        icancloud_Base* process;
        int uid;
        int toGateIdx;
    };
    typedef processRunning_t processRunning;

    vector<processRunning*> processesRunning;

    struct pendingJob_t{
        int messageId;
        icancloud_Base* job;
    };
    typedef pendingJob_t pendingJob;

    vector<pendingJob*>pendingJobs;

		/** Input gates from Apps. */
	    cGateManager* fromAppGates;
		
		/** Input gate from Memory. */
		cGate* fromMemoryGate;
		
		/** Input gate from Net. */
		cGate* fromNetGate;
		
		/** Input gate from CPU. */
		cGate* fromCPUGate;		

		/** Output gates to Apps. */
		cGateManager* toAppGates;
		
		/** Input gate to Memory. */
		cGate* toMemoryGate;
		
		/** Input gate to Net. */
		cGate* toNetGate;
		
		/** Input gate to CPU. */
		cGate* toCPUGate;

		/** Amount of memory free */
	    double memoryFree_KB;
	    double totalMemory_KB;

	    /** Amount of storage capacity free */
	    double storageFree_KB;
	    double totalStorage_KB;

protected:

	   /**
	    * Destructor.
	    */    		
	    ~AbstractSyscallManager();
	  	        			  	    	    
	   /**
	 	*  Module initialization.
	 	*/
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        
	    
	   /**
	 	* Module ending.
	 	*/ 
	    virtual void finish() override;

public:

	   /**
		* Get the outGate ID to the module that sent <b>msg</b>
		* @param msg Arrived message.
		* @return. Gate Id (out) to module that sent <b>msg</b> or NOT_FOUND if gate not found.
		*/ 
		virtual cGate* getOutGate (cMessage *msg) override;

	   /**
		* Process a self message.
		* @param msg Self message.
		*/
		virtual void processSelfMessage (cMessage *msg) override;

       /**
        * Process a response message.
        * @param sm Request message.
        */
        virtual void processResponseMessage (Packet *pkt) override;


	   /**
		* Process a request message.
		* @param sm Request message.
		*/
		virtual void processRequestMessage (Packet *pkt) override = 0;

        /*******************************************************************************
         *                          Operations with storage
         *******************************************************************************/


        /*
         * This method create a process at system
         */
        virtual int createProcess(icancloud_Base* job, int uid) = 0;

        /*
         * This method allow the connection from a compute node to a storage node. It is invoked by the cloud manager and the responsible
         * of the data storage connection is the remote storage app allocated at target (data storage) node.
         */
        virtual void connectToRemoteStorage (vector<string> destinationIPs, string fsType, int localPort, int uId, int pId, string ipFrom, int jobId) = 0;

        /*
         * This method creates a listen connection in the data storage node, in order to wait until the compute node (which hosts
         * the vm) completes the connection. Host node requests a 'remote storage connection' against the storageLocalPort of the storage node.
         */
        virtual void createDataStorageListen (int uId, int pId) = 0;

        /*
         * This method load remote files and configure the filesystems for the account of the user into the remote storage nodes selected by the scheduler of the cloud manager
         */
        virtual void setRemoteFiles ( int uId, int pId, int spId, string ipFrom, vector<preload_T*> filesToPreload, vector<fsStructure_T*> fsPaths) = 0;

        /*
         * This method delete the fs and the files from the remote storage nodes that have allocated files from him.
         */
        virtual void deleteUserFSFiles( int uId, int pId) = 0;

        /*
         * This method create files into the local filesystem of the node.
         */
        virtual void createLocalFiles ( int uId, int pId, int spId, string ipFrom, vector<preload_T*> filesToPreload, vector<fsStructure_T*> fsPaths) = 0;

        /*
         * This method returns the number of user processes running at node
         */
        int getNumProcessesRunning(){return processesRunning.size();};

        /*
         * This method returns the user process running at node
         */
        virtual icancloud_Base* getProcessRunning(int index) {return (*(processesRunning.begin() + index))->process;};

        /*
         *  This method returns the state of the node
         */
        virtual string getState () = 0;

        /*
         *  This method changes the state of the node calling to the states application
         */
        virtual void changeState(const string & newState) = 0;

        // --------------------------------------------------------------------------/
        // ----------------- Operations with attributes of the node -----------------/
        // --------------------------------------------------------------------------/

        //------------------ To operate with the physical attributes of the node -------------
               double getFreeMemory (){return memoryFree_KB;};
               double getFreeStorage (){return storageFree_KB;};

         // Setters
               void setFreeMemory (double newValue){memoryFree_KB = newValue;totalMemory_KB = newValue;};                                 // Returns the free memory of the node
               void setFreeStorage (double newValue){storageFree_KB = newValue;totalStorage_KB = newValue;};                               // Returns the free storage of the node

               void memIncrease(double size){memoryFree_KB += size;};
               void memDecrease(double size){memoryFree_KB -= size;};

               void storageIncrease(double size){storageFree_KB += size;};
               void storageDecrease(double size){storageFree_KB -= size;};
        /*
         *  this method returns the user id from a job id given as parameter
         */
        int searchUserId(int jobId);

        /*
         * This method removes from the control structure the process using the parameters to search it
         */
        virtual void removeProcess(int pId) = 0;

        /*
         * This method removes from the control structure the process using the parameters to search it
         */
        void removeAllProcesses();

        /*
         * Setter for the manager
         */
        virtual void setManager(icancloud_Base* manager) = 0;

        /*
         * This method will allocate the job after timeToStart seconds
         */
        void allocateJob(icancloud_Base* job, simtime_t timeToStart, int uId);

        /*
         * Check if a process is running at system
         */
        bool isAppRunning(int pId);

protected:
        /*
         *  search and delete from structure the job and the uid.
         */
        icancloud_Base* deleteJobFromStructures(int jobId);

        void resetSystem (){memoryFree_KB = totalMemory_KB; storageFree_KB = totalStorage_KB;};


};

} // namespace icancloud
} // namespace inet

#endif
