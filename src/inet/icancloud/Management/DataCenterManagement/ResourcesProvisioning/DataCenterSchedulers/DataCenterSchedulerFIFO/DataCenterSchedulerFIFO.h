
/**
 * @class interface DataCenterSchedulerFIFO DataCenterSchedulerFIFO.h DataCenterSchedulerFIFO.cc
 *
 * This is an scheduler FIFO.
 * This scheduler select the first job allocated into the waitingQueue.
 * It selects one node to execute the job. The first job that it found in the user defined set.
 * The data center cloud scheduler is not completed. It does not run properly.
 *
 * @authors: Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 */

#ifndef DC_SCHEDULER_FIFO_H_
#define DC_SCHEDULER_FIFO_H_

#include  "inet/icancloud/Management/DataCenterManagement/ResourcesProvisioning/DataCenterSchedulerInterface.h"

namespace inet {

namespace icancloud {


class DataCenterSchedulerFIFO : public DataCenterSchedulerInterface{

	protected:

        int initial_max_storage_per_node;
        double minimum_percent_storageNode_free;

        //Flags To control the energy printing
            bool printNodePowerConsumption;        // print Node instant consumption
            bool printNodeEnergyConsumed;          // print Node energy consumed
            bool printComponentsPowerConsumption;  // print main subsytems instant consumption
            bool printComponentsEnergyConsumed;    // print main subsytems instant consumption
            bool printDataCenterPowerConsumption;  // print all the data-center energyConsumed
            bool printDataCenterEnergyConsumed;    // print all the data-center instant consumption

            double dc_EnergyConsumed;  // Tostorage the total data center energy consumed.

 // -------------------------------------------------------------------------------
 // --------------------- implementation of virtual methods -----------------------
 // -------------------------------------------------------------------------------

        /*
         * Initialization of the internal and/or auxiliar structures defined by user in the scheduler internals.
         */
		void setupScheduler();

        /*
         * This method is responsible for scheduling cloud manager. Depending on the operation of the request the scheduling invokes different methods.
         * This method block the incoming of requests, deriving them to a temporal queue until this method finishes.
         * Before this method finishes, it program a new alarm to invoke a new scheduling event after 1 sec. If this interval
         * is reduced, it is possible that it produces starvation.
         */
        void schedule ();

        /*
         * This method returns a node depending on the resources requested.
         */
		virtual AbstractNode* selectNode (AbstractRequest* req);

        /*
         * This method returns the direction(s) of the node(s) where vm's is going to use for
         * storaging data. The parameter fsType is for selecting NFS, PVFS, or another type of
         * file system requested by user
         */
        vector<AbstractNode*> selectStorageNodes (AbstractRequest* st_req);

        /*
         * This method returns true if the scheduler decides to shutdown the node if it is in IDLE state.
         */
        bool shutdownNodeIfIDLE(){return true;};

        /*
         *  This method is invoked when a shutdown request is going to be executed. It has to return a
         *  vector with the nodes where the vm has remote storage, or an empty vector if it only has
         *  local storage. It is the scheduler responsibility, the managing and control of where are the vms allocated
         *  and which nodes it is using for remote storage.
         */
         vector<AbstractNode*> remoteShutdown (AbstractRequest* req);

        /*
         * This method is invoked by CloudManager when a virtual machine has finalized.
         */
		void freeResources (int uId, int pId, AbstractNode* computingNode);

        /*
         * This method defines the data that will be printed in 'logName' file at 'OUTPUT_ DIRECTORY'
         * if the printEnergyToFile and printEnergyTrace are active
         */
		void printEnergyValues();

		/*
		 *  This method is invoked before to finalize the simulation
		 */
		void finalizeManager();

  private:

        /*
         * This method returns the nodes where the userd id has remote storage
         */
        vector<AbstractNode*> getStorageNodes (string userID);

};

} // namespace icancloud
} // namespace inet

#endif
