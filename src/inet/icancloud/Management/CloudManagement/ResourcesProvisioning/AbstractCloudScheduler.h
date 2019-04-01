
/**
 * @class interface AbstractCloudScheduler.h
 *
 * This class has all the abstract methods to be implemented at the core of a scheduler.
 * Only encoding those methods new schedules with different behaviours can be produced.
 *
 * @authors: Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2011-29-08
 */

#ifndef __CLOUD_SCHEDULER_BASE_H_
#define __CLOUD_SCHEDULER_BASE_H_

#include <omnetpp.h>
#include "inet/icancloud/Management/CloudManagement/CloudManager/AbstractCloudManager.h"

namespace inet {

namespace icancloud {




class AbstractCloudScheduler : public AbstractCloudManager{

	protected:

        /*
        * Initialization of internal structures defined by user at schuler.
        */
        void setupScheduler() = 0;

        /*
        * This method is the scheduling core. Depending on the operation of
        * the request (USER_START_VMS, USER_START_VMS_REENQUEUE, USER_REMOTE_STORAGE, USER_LOCAL_STORAGE, USER_SHUTDOWN_VM)
        * The scheduling invokes different methods.
        * This method block the incoming of requests, deriving them to a temporal queue until this method finishes.
        * The time between the echeduling events are setted by time_between_scheduling_events as an alarm at AbstractCloudManager.
        * If this interval is too short, it is possible that it produces starving.
        */
        void schedule () = 0;

        /*
        * This method returns the node where the virtual machine given as parameter (vm) is going to be allocated.
        */
        AbstractNode* selectNode (AbstractRequest* req) = 0;

        /*
        * This method returns a set of node(s) that vm's is going to use for
        * storaging data. The parameter fsType is for selecting NFS, PVFS, or another type of
        * file system requested by user
        */
        vector<AbstractNode*> selectStorageNodes (AbstractRequest* st_req) = 0;

        /*
        * This method returns true if the scheduler decides to shutdown the node after the vm closing.
        * It only will switch off the node if the vm as parameter is the only one at the node
        */
        bool shutdownNodeIfIDLE() = 0;

        /*
        *  This method is activated when a shutdown request is going to be executed. It has to return a
        *  vector with the nodes where the vm has remote storage, or an empty vector if it only has
        *  local storage. It is the scheduler responsibility, the managing and control of where are the vms allocated
        *  and which nodes it is using for remote storage.
        */
        vector<AbstractNode*> remoteShutdown (AbstractRequest* req) = 0;

        /*
        * This method is invoked when a virtual machine has finalized.
        */
        void freeResources (int uId, int pId, AbstractNode* computingNode) = 0;

        /*
        * This method defines the data that will be printed in 'logName' file at 'OUTPUT_ DIRECTORY'
        * if the printEnergyToFile and printEnergyTrace are active
        */
        void printEnergyValues() = 0;

        /*
        *  This method is invoked before to finalize the simulation
        */
        void finalizeScheduler() = 0;


};

} // namespace icancloud
} // namespace inet

#endif /* __CLOUD_SCHEDULER_BASE_H_ */
