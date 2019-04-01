
// Module that implements a Node with its virtualization layer (hypervisor).
//
// This class models a node with an hypervisor. It is able to execute applications at node and virtual machines
//  requested by tenants.
//
// @author Gabriel González Castañé
// @date 2014-12-12

#ifndef _NODEVM_H_
#define _NODEVM_H_

#include "inet/icancloud/Base/VMID/VMID.h"
#include "inet/icancloud/Virtualization/VirtualMachines/VMManagement/VmMsgController/VmMsgController.h"
#include "inet/icancloud/Virtualization/Hypervisor/Hypervisors/Hypervisor.h"
#include "inet/icancloud/Architecture/Node/Node/Node.h"
#include "inet/icancloud/Management/CloudManagement/CloudManager/AbstractCloudManager.h"

namespace inet {

namespace icancloud {


class Hypervisor;

class NodeVL : public Node {

protected:

    Hypervisor* hypervisor;
    vector <VMID*> instancedVMs;						// VMs instanced in the node

public:

    ~NodeVL();

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    
    virtual void finish() override;

		void freeResources (int pId, int uId);

		//------------------ To operate with instanced VMs -------------

		bool testLinkVM (int vmCPUs, int vmMemory, int vmStorage, int vmNetIF, string vmTypeID, int uId, int pId);

		/*
		 * Link VMs has to be invoked when an user request petition is attended.
		 */
		void linkVM  (cGate** iGateCPU,cGate** oGateCPU, cGate* iGateMemI,cGate* oGateMemI,cGate* iGateMemO,
		              cGate* oGateMemO, cGate* iGateNet,cGate* oGateNet,cGate* iGateStorage,cGate* oGateStorage,
                      int numCores, string virtualIP, int vmMemory, int vmStorage, int uId, int pId);

		/*
		 * Unlink VM, as to be invoked when the VM is completely deleted.
		 */
		void unlinkVM (int vmNumCores, int vmMemory, int vmStorage, string virtualIP, int uId, int pId);

		/*
		 * Returns the VMs structure size.
		 */
		int getNumOfLinkedVMs ();

		/*
		 * This method returns the vm identifier allocated at 'index' position
		 */
		int getVMID (int index);

		/*
		 * This method set the vm pointer into the node to allow the access from the node to the virtual machines allocated in it.
		 */
		void setVMInstance(VM* vmPtr);

		/*
		 * This method returns the vm pointer with an identifier that matches with the given parameter vmID
		 */
		VM* getVMInstance (int pId, int uId);

		/*
		 * This method returns the num of vm allocated in the node.
		 */
		int getNumVMAllocated(){return instancedVMs.size();};


	    int getNumProcessesRunning() override {return (os->getNumProcessesRunning() + getNumVMAllocated());};

		// ------------------- Initial notifications to Cloud Manager -------------------

        void notifyManager (Packet *) override;

        void setManager(icancloud_Base* manager) override;

        void initNode () override;

        cModule* getHypervisor();

        //------------------ To operate with the state of the node -------------
            virtual void turnOn () override;                                                     // Change the node state to on.
            virtual void turnOff () override;                                                    // Change the node state to off.

        // ------------------ Methods to migrate virtual machines ----------

        // This method is to create a conection between two nodes to migrate virtual machines
        void createListenToMigrate(){}
        void connectToMigrate (string destinationIP, string localIP, VM* vm,  bool liveMigration, int dirtyingRate){}


};

} // namespace icancloud
} // namespace inet

#endif /* _NODE_H_ */
