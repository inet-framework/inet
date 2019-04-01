
/**
 * @class interface CloudSchedulerRR CloudSchedulerRR.h CloudSchedulerRR.cc
 *
 * This is an scheduler following a policy Round Robing.
 * This scheduler select the first job allocated into the waitingQueue.
 * It selects a node from the structure for allocate a virtual machine each time. If all the
 * nodes has a virtual machine, the scheduler repeats at same order the allocation.
 *
 * @authors: Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 */

#ifndef SCHEDULER_RR_H_
#define SCHEDULER_RR_H_

#include "inet/icancloud/Management/CloudManagement/ResourcesProvisioning/AbstractCloudScheduler.h"

namespace inet {

namespace icancloud {


class CloudSchedulerRR : public AbstractCloudScheduler{

protected:

    double minimum_percent_storageNode_free;
    int maximum_number_of_processes_per_node;

    int currentNodeIndex;
    int currentNodeType;

     //Flags To control the energy printing
         bool printNodePowerConsumption;        // print Node instant consumption
         bool printNodeEnergyConsumed;          // print Node energy consumed
         bool printComponentsPowerConsumption;  // print main subsytems instant consumption
         bool printComponentsEnergyConsumed;    // print main subsytems instant consumption
         bool printDataCenterPowerConsumption;  // print all the data-center energyConsumed
         bool printDataCenterEnergyConsumed;    // print all the data-center instant consumption

         double dc_EnergyConsumed;  // For storing the total data center energy consumed.


     /*
      * Module initialization
      */
         virtual void initialize(int stage) override;
         virtual int numInitStages() const override { return NUM_INIT_STAGES; }
         

// -------------------------------------------------------------------------------
// --------------------- implementation of virtual methods -----------------------
// -------------------------------------------------------------------------------

    /*
    * Initialization of internal structures defined by user at schuler.
    */
    void setupScheduler() override;

    /*
    * This method is the scheduling core. Depending on the operation of
    * the request (USER_START_VMS, USER_START_VMS_REENQUEUE, USER_REMOTE_STORAGE, USER_LOCAL_STORAGE, USER_SHUTDOWN_VM)
    * The scheduling invokes different methods.
    * This method block the incoming of requests, deriving them to a temporal queue until this method finishes.
    * The time between the echeduling events are setted by time_between_scheduling_events as an alarm at AbstractCloudManager.
    * If this interval is too short, it is possible that it produces starving.
    */
    void schedule() override;

    /*
    * This method returns the node where the virtual machine given as parameter (vm) is going to be allocated.
    */
    AbstractNode* selectNode (AbstractRequest* req) override;

    /*
    * This method returns a set of node(s) that vm's is going to use for
    * storaging data. The parameter fsType is for selecting NFS, PVFS, or another type of
    * file system requested by user
    */
    vector<AbstractNode*> selectStorageNodes (AbstractRequest* st_req) override;

    /*
    * This method returns true if the scheduler decides to shutdown the node after the vm closing.
    * It only will switch off the node if the vm as parameter is the only one at the node
    */
    bool shutdownNodeIfIDLE() override {return true;};

    /*
    *  This method is activated when a shutdown request is going to be executed. It has to return a
    *  vector with the nodes where the vm has remote storage, or an empty vector if it only has
    *  local storage. It is the scheduler responsibility, the managing and control of where are the vms allocated
    *  and which nodes it is using for remote storage.
    */
    vector<AbstractNode*> remoteShutdown (AbstractRequest* req) override;

    /*
    * This method is invoked when a virtual machine has finalized.
    */
    void freeResources (int uId, int pId, AbstractNode* computingNode) override;

    /*
    * This method defines the data that will be printed in 'logName' file at 'OUTPUT_ DIRECTORY'
    * if the printEnergyToFile and printEnergyTrace are active
    */
    void printEnergyValues() override;

    /*
    *  This method is invoked before to finalize the simulation
    */
    void finalizeScheduler() override;

private:

    /*
    * This method, selects the heterogeneous node set that is able to allocate an amount of memory and a set of cores..
    */
    int selectNodeSet  (const string &setName, int vmcpu, int vmmemory);



};

} // namespace icancloud
} // namespace inet

#endif

