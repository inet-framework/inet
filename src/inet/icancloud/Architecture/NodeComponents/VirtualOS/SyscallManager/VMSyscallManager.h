#ifndef __VMSYSCALLMANAGER_H_
#define __VMSYSCALLMANAGER_H_

#include "inet/icancloud/Virtualization/VirtualMachines/VMManagement/VmMsgController/VmMsgController.h"
#include "inet/icancloud/Architecture/NodeComponents/OperatingSystems/SyscallManager/AbstractSyscallManager.h"
#include "inet/icancloud/Users/Profiles/CloudUser/AbstractCloudUser.h"

namespace inet {

namespace icancloud {

/**
 * @class SyscallManager SyscallManager.h "Architecture/NodeComponents/OperatingSystems/SyscallManager/NodeSyscallManager/SyscallManager.h"
 *   
 * SyscallManager to be connected inside a VM. It has not allow to operate
 * directly with the nodes neither with energy states.
 * 
 * @author Alberto N&uacute;&ntilde;ez Covarrubias
 * @date 2009-03-11
 * @author - cleanup and rewrited by Gabriel González Castañé -
 * @date october 2014
 */

class AbstractCloudUser;

class VMSyscallManager : virtual public AbstractSyscallManager{

protected:

        string state;                         // defined as MACHINE_STATE_IDLE || MACHINE_STATE_RUNNING || MACHINE_STATE_OFF
	        	
		/** Pointer to migration controller*/
		VmMsgController* mControllerPtr;

		/** The manager of a virtualized machine is the user */
		AbstractCloudUser* user;

	   /**
	    * Destructor.
	    */    		
	    ~VMSyscallManager();
	  	        			  	    	    
	   /**
	 	*  Module initialization.
	 	*/
	    virtual void initialize(int stage) override;
	    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

        
	    
	   /**
	 	* Module ending.
	 	*/ 
	    void finish() override;

	   /**
		* Process a request message.
		* @param sm Request message.
		*/
		void processRequestMessage (Packet *) override;

public:

        /*
         * Unlink and destroy an application from the system
         */
        void removeProcess(int pId) override;

        /*
         * This method create a process at the system linking it to the system
         */
        int createProcess(icancloud_Base* job, int uid) override;



        /*******************************************************************************
         *                          Operations with storage
         *******************************************************************************/

        /*
         * This method allow the connection from a compute node to a storage node. It is invoked by the cloud manager and the responsible
         * of the data storage connection is the remote storage app allocated at target (data storage) node.
         */
        void connectToRemoteStorage (vector<string> destinationIPs, string fsType, int localPort, int uId, int pId, string ipFrom, int jobId) override;

        /*
         * This method creates a listen connection in the data storage node, in order to wait until the compute node (which hosts
         * the vm) completes the connection. Host node requests a 'remote storage connection' against the storageLocalPort of the storage node.
         */
        void createDataStorageListen (int uId, int pId) override;

        /*
         * This method load remote files and configure the filesystems for the account of the user into the remote storage nodes selected by the scheduler of the cloud manager
         */
        void setRemoteFiles ( int uId, int pId, int spId, string ipFrom, vector<preload_T*> filesToPreload, vector<fsStructure_T*> fsPaths) override;

        /*
         * This method delete the fs and the files from the remote storage nodes that have allocated files from him.
         */
        void deleteUserFSFiles( int uId, int pId) override;

        /*
         * This method create files into the local filesystem of the node.
         */
        void createLocalFiles ( int uId, int pId, int spId, string ipFrom, vector<preload_T*> filesToPreload, vector<fsStructure_T*> fsPaths) override;

        /*
         *  This method returns the state of the node
         */
        string getState () override;

        /*
         *  This method changes the state of the node calling to the states application
         */
        void changeState(const string & newState) override;

        /*
         * Setter for the manager
         */
        void setManager(icancloud_Base* manager) override;



};

} // namespace icancloud
} // namespace inet

#endif
