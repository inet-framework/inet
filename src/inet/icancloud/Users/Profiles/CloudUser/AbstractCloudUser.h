/**
 * @class AbstractCloudUser AbstractCloudUser.cc AbstractCloudUser.h
 *
 * This class is responsible for the model of a user into the icancloud.
 * The functionality is to identify who is launching a job, and where.
 * An user storage a set of VMs of each kind that he pays for.
 *
 * @authors Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2012-06-10
 */

#ifndef _ABSTRACT_CLOUD_USER_H_
#define _ABSTRACT_CLOUD_USER_H_


#include "inet/icancloud/Virtualization/VirtualMachines/VM.h"
#include "inet/icancloud/Base/Parser/cfgMPI.h"
#include "inet/icancloud/Architecture/NodeComponents/VirtualOS/SyscallManager/VMSyscallManager.h"
#include "inet/icancloud/Management/MachinesStructure/MachinesMap.h"
#include "inet/icancloud/Users/AbstractUser.h"
#include "inet/icancloud/Base/Request/RequestVM/RequestVM.h"
#include <fstream>

namespace inet {

namespace icancloud {


class VM;
class VMSyscallManager;

class AbstractCloudUser: virtual public AbstractUser {

protected:

	/** The set of virtual machines that the user has requested for**/
	    MachinesMap* machinesMap;

    // Print results
        bool printVMs;
        bool printJobs;


    // This structure is for allocate the resources that a user wants/has
        struct userVmType_t{
            elementType* type;
            int quantity;
        };

        typedef userVmType_t userVmType;

        vector<userVmType*> vmsToBeSelected;

        vector <VM*> wastedVMs;
        /*************************************************************************
         *                      Attributes
         **************************************************************************/

                vector <VM*> vmListToExecute;

            // VMs pending to initialize by user
                int pendingVmsAtStartup;
                vector<VM*> vms_waiting_remote_st;

            // remote connections to storage

            // Vector for start the jobs
                vector<cModule*> pending_jobs;

            // User finalization..
                int pending_shutdown_requests;

		/*******************************************************************************
		 *            Virtual methods to create different users
		 *******************************************************************************/

		// The initialize of the user behavior. Start VMs, all, any .. ?
		virtual void userInitialization () override = 0;

		/*
		 * This method is invoked if it were necessary to realize any operation before the
		 * user leave the cloud system
		 */
		virtual void userFinalization () override = 0;

		/*
		 * This method select virtual machines from the MachinesMap to be started.
		 */
		virtual AbstractRequest* selectVMs_ToStartUp () = 0;

		/*
		 * This method select a set of VMs (or one) to execute the given job as parameter.
		 */
		virtual AbstractRequest* selectResourcesJob (jobBase* job) override = 0;

		/*
		 * This method returns a job of the user list of jobs waiting to be executed at waiting queue.
		 */
		virtual jobBase* selectJob () override = 0;

		/*
		 *  This method is invoked from the application when a job has finished.
		 *  It is useful to extract values from the job, or the timings of the executions.
         *  The method notify_UserJobHasFinished is responsible for deleting the job, move it from running queue to finish queue
         *  and call to schedule.
         */

		virtual void jobHasFinished (jobBase* job) override = 0;

		/*
		 * When the CloudManager attends a request and creates the VMs, it notifies this fact to
		 * the user with request attended. The parameters are a vector with pointers created to the VMs modules and
		 * the virtual machine type which they belong.
		 */
		virtual void requestAttended (AbstractRequest* req) override = 0;

		/*
		 * When a request operation is unknown or it provokes an exception, this method
		 * will be called by CloudManager.
		 */
		virtual void requestErrorDeleted (AbstractRequest* req) override = 0;

		/*
		 * The order of events that defines the behavior of each type of user.
		 */
        virtual void schedule() override = 0;

        /*******************************************************************************
         *                              END Virtual methods
         *******************************************************************************/
public:
        /*
         * Destructor.
         */
        virtual ~AbstractCloudUser ();

        /*
         * Module Initialization
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

        /*
         * This method finalizes the user structures.
         * It Supress all the Jobs from the Waiting Queue to the Finish Queue.
         *  Move those jobs to the finish queue
         *
         * If everything is correct, return true.
         * Otherwise return false without perform any action against jobs.
         *
         */
        virtual bool finalizeUser() override;


        /*******************************************************************************
         *                             methods to operate with vms
         *******************************************************************************/
        /*
         * To add a vm to the users virtual machine set when it is parsed from the parameters defined in omnetpp.ini
         */
        void createVMSet (int vmsQuantity, int numCores, int memorySizeMB, int storageSizeGB, int numNetIF, string vmSelectionType);

        /*
         * This method increases the maximum limit of resources(vms) of the user.
         */
        void increaseVMSet (int vmsQuantity, string vmType);

        /*
         * To decrease the resources of the user suppressing the VM with id = vmID
         */
        void decreaseVMSet (string vmType, int vmsQuantity);

        /*
         * To get the type of a machines to be started by index
         */
        elementType* getMachinesType(int index){return ((*(vmsToBeSelected.begin() +index))->type);};

        /*
         * test the number of vms to be started from a given type
         */
        int machinesLeft (int index);

        /*
        * This method will request to the manager for the machines at vector given as parameter
        * The method creates a request and send it to the cloud manager.
        * The request could not be attended immediately by CloudManager.
        */
        void startVMs (AbstractRequest* vmSet);

        /*
        * This method is invoked to shutdown a VM and destroy its module.
        * The method creates a request and send it to the cloud manager.
        * The request could not be attended immediately by CloudManager.
        */
        void shutdown_VM (VM* vm);

        /*
         * Test if all vms have been shutdown
         */
        bool checkAllVMShutdown();


        /*******************************************************************************
         *                             methods to operate jobs
         *******************************************************************************/

        /*
         * This method connect the cmodule from the created jobs list to the vmInstances defined in the attribute of the job.
         * @Param: The cloud job.
         */
        int allocateJob(jobBase* job) override;

        /*
        * This method deletes the cloudJob given as parameter, which it is allocated in the vm given also as parameter.-
        */
        void deleteJobVM (VM* vm, UserJob* job);

        /*
        * This method delete all user jobs that are executing into a virtual machine
        */
        void deleteAllJobsVM (VM* vm);


        /*
         * This method starts dinamically an application.
         * @Param: The job that is going to execute. The set of machines to execute the job is embedded into
         *         the job.
         * @Param: The vm where the job is allocated.
         *         If it is nullptr, the job will be searched in the waiting Queue.
         * @Param: The vector where the job is going to be allocated.
         *         If it is nullptr, the job will be moved to the running Queue.
         * @Param: The position of the destination vector where the job is going to allocate.
         */
        void start_up_job_execution (VM* vmToExecute, UserJob* job, JobQueue* qSrc, JobQueue* qDst, int qDst_pos = -1);

        /*
         * This method execute the jobs assigned for the vms at the scheduling method.
         * The are allocated at the waiting for remote storage queue (private). When the
         * FS has been created for the concrete job at the vm, this method starts the pending jobs.
         */
        void executePendingJobs();

        /*
         * This method check if there are any vms waiting for remote storage or shutting down.
         */
        bool hasPendingRequests() override;

        /*
         * This method crates the fs for a job. Creates the entry and the folder structures.
         * In addition is possible to preload files defined inside the job structure.
         * The spId parameter is subprocessId, that is the id of a virtual machine if the
         * job will be executed inside.
        */
        vector<StorageRequest*> createFSforJob (jobBase* job, string opIp, string nodeSetName, int nodeId, int optionalID = -1) override;


        /*******************************************************************************
         *                      methods to notify to user from external events
         *******************************************************************************/

        /*
         * This method is invoked by system manager / scheduler when a job has finished.
         *
         * It removes from the running queue and insert into the finish queue and then
         * call to virtual jobHasFinished().
         * The method finalizes calling to the schedule method.
         */
        virtual void notify_UserJobHasFinished (jobBase* job) override;

        /*
         * This method is invoked when a request has been attendeed.
         */
        void notify_UserRequestAttendeed (AbstractRequest* req) override;

        /*
         * This methods are invoked when a request has provoked an error.
         */
        void notify_UserRequestError (AbstractRequest* req) override {requestAttended (req);};


        /* ---------------------------------------------------------------------------
         * ---------------- OPERATIONS WITH VIRTUAL MACHINES -------------------------
         * ---------------------------------------------------------------------------
         */

        /*
         * This method erase from the structure vms_to_initialize when the operation will be equal to  USER_START_VMS.
         * If the operation is equal to USER_XXX_STORAGE, the deletion has been performed using the method vms_waiting_remote_st.
         */
        bool eraseVMFromControlVector(VM* vm, int operation);

        /*
         * This method search in the vms_waiting_remote_st structure and returns true if the vm given as
         * parameter is founded.
         */
        bool vmHasStorageRequests(VM* vm);

        /*
         * This method returns the size of the vms_waiting_remote_st structure.
         */
        int vms_waiting_for_remote_storage();

        /*
         * This method returns the size of the vms_to_initialize structure.
         */
        int vms_waiting_for_initialization();


        VM* searchVmByPid(int pid);

        /*
         * this method send the request to the cloud manager
         */
        void send_request_to_manager (AbstractRequest* req) override;

        /*
         * this method delete the vms (if there is any) at wasted vms vector;
         */
        void deleteAllVMs();

private:
        vector<UserJob*> jobToDelete;

};

} // namespace icancloud
} // namespace inet

#endif
