#ifndef __ABSTRACTMANAGER_BASE_H_
#define __ABSTRACTMANAGER_BASE_H_

#include "inet/icancloud/Management/CloudManagement/Base/AllocationManagement.h"
#include "inet/icancloud/Base/Parser/cfgCloud.h"
#include "inet/icancloud/Virtualization/VirtualMachines/VM.h"
#include "inet/icancloud/Architecture/Node/NodeVL/NodeVL.h"
#include "inet/icancloud/Base/Request/RequestVM/RequestVM.h"

namespace inet {

namespace icancloud {


class CfgCloud;
class RequestVM;

class AbstractCloudManager : virtual public AllocationManagement{

protected:

    /*
     * Migration attributes
     */
    // If the vm migrations stop the virtual machine or not
    bool liveMigration;

    // The dirtying rate in KB
    int dirtyingRate;

    struct MigrationStructure{
            AbstractNode* host;
            AbstractNode* target;
            VM* vm;
    };

    vector<MigrationStructure*> migrationVector;

    bool migrationActive;

    vector <RequestVM*> reqPendingToDelete;

protected:


		/*
		 * Destructor
		 */
		virtual ~AbstractCloudManager();

       /*
        * Module initialization of parameters. Call to getconfig to obtain the cloud parameters,
        * and setup the scheduler.
        */
	    virtual void initialize(int stage) override;
	    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        

		/*
		 * This method initialize a new cloudManager
		 */
		void initManager (int totalNodes = -1) override;

        void finalizeManager() override;

protected:
        /************************************************************
         *               To be implemented at scheduler
         ************************************************************/
            /*
             * Initialization of the internal and/or auxiliar structures defined by programmer in scheduler internals.
             */
            virtual void setupScheduler() override = 0;

            /*
             * This method is responsible for scheduling cloud manager. Depending on the operation of
             * the request (USER_START_VMS, USER_START_VMS_REENQUEUE, USER_REMOTE_STORAGE, USER_LOCAL_STORAGE, USER_SHUTDOWN_VM)
             * The scheduling invokes different methods.
             * This method block the incoming of requests, deriving them to a temporal queue until this method finishes.
             * Before this method finishes, it program a new alarm to invoke a new scheduling event after 1 sec. If this interval
             * is reduced, it is possible that it produces inanition.
             */
            virtual void schedule () override = 0;

            /*
             * This method returns the node where the virtual machine given as parameter (vm) is going to be allocated.
             */
            virtual AbstractNode* selectNode (AbstractRequest* req) override = 0;

            /*
             * This method returns the node(s) that vm's is going to use for
             * storaging data. The parameter fsType is for selecting NFS or PFS.
             */
            virtual vector<AbstractNode*> selectStorageNodes (AbstractRequest* st_req) override = 0;

            /*
             * This method returns true if the scheduler decides to shutdown the node after the vm closing.
             * It only will switch off the node if the vm as parameter is the only one at the node
             */
            virtual bool shutdownNodeIfIDLE() override = 0;

            /*
             *  This method is invoked when a shutdown request is going to be executed. It has to return a
             *  vector with the nodes where the vm has remote storage, or an empty vector if it only has
             *  local storage. It is the scheduler responsibility, the managing and control of where are the vms allocated
             *  and which nodes it is using for remote storage.
             */
            virtual vector<AbstractNode*> remoteShutdown (AbstractRequest* req) override = 0;

            /*
             * This method is invoked by CloudManager when a virtual machine has finalized.
             */
            virtual void freeResources (int uId, int pId, AbstractNode* computingNode) override = 0;

            /*
             *  This method is invoked before to finalize the simulation
             */
            virtual void finalizeScheduler() = 0;

            /*
             * This method defines the data that will be printed in 'logName' file at 'OUTPUT_ DIRECTORY'
             * if the printEnergyToFile and printEnergyTrace are active
             */
            virtual void printEnergyValues() override = 0;


            VM* getVmFromUser(int uId, int pId);
            AbstractCloudUser* getUserFromId(int uId);

	    // -------------------------------------------------------------------------------
	    // --------------------- Operations "actuators" from requests --------------------
	    // -------------------------------------------------------------------------------

	        /*
	         * this method select, allocate the vm on a node and return it to be started by user
	         */
	        bool request_start_vm (RequestVM* req);

	        /*
	         * This method is invoked to attend a vm shutdown request;
	         */
	        void request_shutdown_vm(RequestVM* req);


		// ------------------------ operations with connections of vms --------------------

	        /*
	         * This method calls to link vm in order to connect the virtual machines to the node.
	         */
	        void linkVM (AbstractNode* Node, VM* vm);

            /*
             * This method calls to unlink VM, to disconnect the virtual machine from the node. If everything is ok,
             * it destroys the virtual machine and free the resources.
             */
            void unlinkVM(AbstractNode* node, VM* vm, bool turnOff = false);

            /*
             * This method closes all VM connections
             */
            void closeVMConnections (vector<AbstractNode*> nodes, VM* vm);

public:
             /*
              * This method is called from a node to notify to the manager that an operation have been performed
              */

            virtual void notifyManager(Packet *) override;

private:

         // ----------------------------------VM INTERNALS -------------------------------
            /*
             * When a VM is created, the scheduler has to call to this method, link_VMtoNode, to
             * link the vm to the hypervisor's node, physically. With the parameters Node and VM link also the
             * objects, to search if it were necessary where is allocated a vm or which vm a node has allocated.
             *
             * This method returns -1 if it exists any error. If all is correct, it returns 0;
             */
            void linkVMInternals (AbstractNode* node, VM* vm, bool migration);

            /*
             * This method unlink a vm from the node, but it does not finalizes it.
             */
            void unlinkVMInternals (AbstractNode* node, VM* vm, bool migration);

            /*
             * This method creates and links the vm module to the pointer of VM.
             * The module created needs to be linked to a Node and needs to be loaded with an application
             * (by user side) to be executed properly.
             */
            VM* create_VM(VM* vmImage, string vmName, cModule* parent);

        //----------------------------- Notifications from the system----------------------------


	    /*
	     * This is invoked to notify that the storage connection has been performed successful
	     */
	    void notifyStorageConnectionSuccesful (int uId, int pId, int spId);

	    /*
         * This method is invoked to notify that the file system of user has been deleted
         */
        void notifyFSFormatted(int uId, int pId, bool turnOffNode = false);

        /*
         *  This method is invoked by CloudManager when a vm is shutdown.
         */
        void notify_shutdown_vm (int uId, int pId, AbstractNode* node);

        /*
         * This method is to notify to the manager that the VMConnections have been closed
         */
        void notifyVMConnectionsClosed (int uId, int pId, bool turnOffNode = true);


		// ---------------------- methods to migrate vm's ------------------------------------

        void migrateVM(AbstractNode* source, AbstractNode* destination, VM* vm);



};

} // namespace icancloud
} // namespace inet

#endif


