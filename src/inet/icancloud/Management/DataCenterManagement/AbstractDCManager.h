/**
 * @class  DataCenterManager.cc DataCenterManager.h
 *
 * This is the abstract data center manager. It is responsible for all the main operations for managing
 * resources from the data center. It has the control for the resources booked by users from the data center.
 *
 * @authors: Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2014-22-06
 */

#ifndef __DATACENTERMANAGER_H_
#define __DATACENTERMANAGER_H_

#include <fstream>
#include "inet/icancloud/Management/NetworkManager/NetworkManager.h"
#include "inet/icancloud/Management/DataCenterManagement/Base/DataCenterAPI.h"
#include "inet/icancloud/Management/DataCenterManagement/Base/StorageManagement.h"
#include "inet/icancloud/Management/DataCenterManagement/Base/RequestsManagement.h"
#include "inet/icancloud/Management/DataCenterManagement/Base/UserManagement.h"
#include "inet/icancloud/Base/Parser/cfgDataCenter.h"
#include "inet/icancloud/Architecture/Node/Node/Node.h"

namespace inet {

namespace icancloud {


class StorageManagement;
class DataCenterAPI;
class UserManagement;

class AbstractDCManager : virtual public DataCenterAPI, virtual public StorageManagement, virtual public UserManagement, virtual public RequestsManagement{

protected:

        /** Configuration of the different node types of the data center */
	    CfgDataCenter* dataCenterConfig;

	    /** To know if the CfgDataCenter has been loaded by a superclass */
	     bool configDone;

		/** Running starting timestamp */
		time_t runStartTime;

		/** Running ending timestamp */
		time_t runEndTime;

	    /** Pointer to manage the network connections between nodes and VMs */
	    NetworkManager* networkManager;

	    /** Message to trace the energy each scale time */
	    cMessage *waitToCapture;

	    /** Message to activate an alarm */
        cMessage *smAlarm;

        /** Message to activate an alarm that log results */
        cMessage *logAlarm;

	    /** Time to start system manager */
		double timeToStartManager;

        /* The time between scheduling events (in seconds) */
        simtime_t timeBetweenScheduleEvents_s;

        /* The time between log the energy values(in seconds) */
        simtime_t timeBetweenLogResults_s;

        /** If the energy results will be printed in a file or not */
        bool printEnergyToFile;

        /** print each schedule event the consumption values of the nodes */
        bool printEnergyTrace;

        /** Name for the file for print energy data */
        string logName;

        /** Output File directory */
        static const string OUTPUT_DIRECTORY;

        // MEMORIZATION
            bool memorization;
            vector<MemoSupport*> cpus;
            vector<MemoSupport*> memories;
            vector<MemoSupport*> storages;
            vector<MemoSupport*> networks;

protected:

        void cfgLoaded(){configDone = true;};
        bool isCfgLoaded(){return configDone;};
       // -------------------------------------------------------------------------------
       // --------------------- virtual methods to be implemented -----------------------
       // -------------------------------------------------------------------------------

       /*
         * Initialization of the internal and/or auxiliar structures defined by user in the scheduler internals.
         */
        virtual void setupScheduler() = 0;

        /*
         * This method is responsible for scheduling  managemetn. Depending on the operation of the request the scheduling invokes different methods.
         * This method block the incoming of requests, deriving them to a temporal queue until this method finishes.
         * Before this method finishes, it program a new alarm to invoke a new scheduling event after 1 sec. If this interval
         * is reduced, it is possible that it produces starvation.
         */
        virtual void schedule () = 0;

        /*
         * This method returns a node depending on the resources requested.
         */
        virtual AbstractNode* selectNode (AbstractRequest* req) = 0;

        /*
         * This method returns the direction(s) of the node(s) where vm's is going to use for
         * storaging data. The parameter fsType is for selecting NFS, PVFS, or another type of
         * file system requested by user
         */
        virtual vector<AbstractNode*> selectStorageNodes (AbstractRequest* st_req) = 0;

        /*
         * This method returns true if the scheduler decides to shutdown the node if it is in IDLE state.
         */
        virtual bool shutdownNodeIfIDLE() = 0;

        /*
         *  This method is invoked when a shutdown request is going to be executed. It is the scheduler responsibility, the managing and control of where are the processes form user has been allocated
         *  and which nodes it is using for remote storage.
         */
         virtual vector<AbstractNode*> remoteShutdown (AbstractRequest* req) = 0;

        /*
         * This method is invoked by manager when a virtual machine has finalized.
         */
         virtual void freeResources (int uId, int pId, AbstractNode* computingNode) = 0;

        /*
         * This method defines the data that will be printed in 'logName' file at 'OUTPUT_ DIRECTORY'
         * if the printEnergyToFile and printEnergyTrace are active
         */
        virtual void printEnergyValues() = 0;

        /*
         *  This method is invoked before to finalize the simulation
         */
        virtual void finalizeManager() override = 0;

		/*
		 * Destructor
		 */
		virtual ~AbstractDCManager();

		/*
        * Module initialization
        */
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        
        /**
         * Module ending.
         */
        void finish() override;

       /**
        * Process a self message.
        * @param msg Self message.
        */
        void processSelfMessage (cMessage *msg) override;;

       /**
        * Process a request message.
        * @param sm Request message.
        */
        void processRequestMessage (Packet *pkt) override;

       /**
        * Process a response message.
        * @param sm Request message.
        */
        void processResponseMessage (Packet *pkt) override;

        /**
        * Get the outGate to the module that sent <b>msg</b>
        * @param msg Arrived message.
        * @return. Gate Id (out) to module that sent <b>msg</b> or NOT_FOUND if gate not found.
        */
        cGate* getOutGate (cMessage *msg) override;

        /*
         * This method initializes a new system manager at last stage
         */
        virtual void initManager (int totalNodes = -1);

        /*
         * This method finalizes a new system manager at first stage.
         * It invokes the method finalize Scheduler from scheduler, turn off all
         * nodes and then, generates an alarm to finalize the simulation.
         */
		void finalizeDCManager();

        /*
         * Initialize the provisioning policy
         */
        void initScheduler();

        /*********************************************************************************************
        ********************** Methods for administrate the resources booked ************************
        *********************************************************************************************/

public:
        /*
        * This method will be used by users in order to book physical resources;
        * If the cores parameter value is -1, the entire node will be booked.
        * The method returns the message id if the reservation has been success. Otherwise, -1
        * value will be returned.
        * The time from a node is booked and the request to operate with the node arrives
        * has to be under 3'
        */
        int bookComputeResources(int uid , int jobID, int nodeSet, int nodeID, int cores = -1);

        /*
        * This method check if the resources will have enough resources to allocate the files of the user
        * given as parameter 'storage'.
        * If the node has enough resources, it will return the connection parameters for the operation.
        * If the file is local, the connection parameters will be ip:localhost and the port:-1.
        * The processID of the connection is the 'ticket' for cancel the book. If the set of operations
        * do not begin in 3', the system will undo all the operations for start the job (book of remote storage
        * if it was the case, and the computational resources).
        */
        connection_T* bookStorageResources(int uid, int jobID, int nodeSet, int nodeID, double storage);

        /*
          * When a problem is detected and a start job is in process, this method free the cores getted by user for
          * the jobId and free storageBook if there was.
          * This process does not delete folder structures or files if the process has been requested before.
          */
         void processRollBack(int uid, int jobId);

         /*
          * This method is called by the Abstract data center manager for adding user id to the network manager
          */
         bool newUser (AbstractUser *user) override;


         virtual void notifyManager(Packet *) override;

protected:

        /*
        * This method allow to administrators and system managers to book the needed cores for a user request.
        */
        void adminBookComputingResources (string nodeSetName, int nodeId, int cores, int uid, int jobId);

        /*
        * This method erase the book from the pendingBook messages due to the user has working with the node
        */
        void eraseComputingBook(int messageID);

        /*
        * This method free the book from the pending book messages freeing the resources.
        */
        void freeComputingBookByTimeout(int messageID);

        /*
         * This method free the book from the pending book messages freeing the resources.
         * Even if the operation has been initialized successfully as if the user does not begin the operation,
         * the method is the same.
         * The reservation has to be deleted when the operation begins or the timeout alarm
         * is activated.
         */
        void freeStorageBook(int messageID = -1, int uid = -1, int jobId = -1, string ip = "", double storage = 0.0);

         /******************************************************************************************************
         *                                 Aux methods for users and requests
         ******************************************************************************************************/

        void userStorageRequest (StorageRequest* st_req, AbstractNode* nodeHost);


        void freeComputeResources(string nodeSet, int nodeIndex, int cores) override;
        /*
         * This method is for checking if the manager(simulation) should finalizes if it have not workload from users
         */
        void checkManagerFinalization();


        /*********************************************************************************************
        ****************** Methods for get the power / energy from physical resources ***************
        *********************************************************************************************/

        /* This method measure the energy of the required system */
        double getEnergyConsumed(bool computeNodes = true, bool storageNodes = true);

        void deleteUser (int userId) override;

        void configureMap (MachinesMap* map);

// Attributes to manage the pending book resources
private:

    struct pendingComputingBookMessage{
        cMessage* message;
        string nodeSet;
        int nodeId;
        int cores;
        int uid;
        int jobID;
    };


    struct pendingStorageBookMessage{
        cMessage* message;
        string ip;
        string nodeSet;
        int nodeId;
        int storage;
        int uid;
        int jobID;
    };

    vector<pendingComputingBookMessage*> pendingComputeBook;
    vector<pendingStorageBookMessage*> pendingStorageBook;

};

} // namespace icancloud
} // namespace inet

#endif


