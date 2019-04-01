
#ifndef DC_SCHEDULER_InterfaceH_
#define DC_SCHEDULER_InterfaceH_

#include "inet/icancloud/Management/DataCenterManagement/AbstractDCManager.h"

namespace inet {

namespace icancloud {


class DataCenterSchedulerInterface : public AbstractDCManager{


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
     virtual void finalizeManager() = 0;
};

} // namespace icancloud
} // namespace inet

#endif
