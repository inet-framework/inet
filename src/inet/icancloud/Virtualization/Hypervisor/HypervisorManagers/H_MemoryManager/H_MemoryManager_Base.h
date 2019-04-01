//
// Class abstract defining the base of the memory controller.
//
// The virtual methods to be implemented at memory manager concrete policy are:
//      - virtual void schedulingMemory(icancloud_Message *msg)
//      - virtual void getVMGateIdx(icancloud_Message *msg)
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//

#ifndef H_MEMORY_MANAGER_BASE_H_
#define H_MEMORY_MANAGER_BASE_H_

#include <omnetpp.h>
#include "inet/icancloud/Base/include/icancloud_debug.h"
#include "inet/icancloud/Base/icancloud_Base.h"
#include "inet/icancloud/Base/include/Constants.h"

namespace inet {

namespace icancloud {


class H_MemoryManager_Base  :public icancloud_Base{

protected:

	/** Maximum number of VMs supported by the hypervisor*/
	int nodeGate;
	int64_t blockSize_KB;
	int64_t memorySize_MB;

    /** Communications with the VM */
	cGateManager* fromVMMemoryI;
	cGateManager* fromVMMemoryO;
	cGateManager* toVMMemoryI;
	cGateManager* toVMMemoryO;

    /** Communications with the VM */
    cGate* fromNodeMemoryI;
    cGate* fromNodeMemoryO;
    cGate* toNodeMemoryI;
    cGate* toNodeMemoryO;

    /*
     * Structure to control the vms linked
     */
    struct vmControl_t{
        int gate;
        int uId;
        int pId;
    };

    typedef vmControl_t vmControl;

    vector<vmControl*> vms;

    // Overhead
    int memory_overhead_MB;
    bool nodeOn;

protected:
   /**
    * Process a self message.
    * @param msg Self message.
    */
    void processSelfMessage (cMessage *msg) override;

   /**
    * Process a request message.
    * @param sm Request message.
    */
    void processRequestMessage (Packet *) override;

   /**
    * Process a response message.
    * @param sm Request message.
    */
    void processResponseMessage (Packet *) override;

    /*
     *
     */
    cGate* getOutGate (cMessage *msg) override;

    /*
     * Destructor
     */
	virtual ~H_MemoryManager_Base();

    /*
     * Module initialization
     */
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    

    /*
     * Module finalization
     */
	void finish() override;


public:

	double getMemoryOverhead() {return memory_overhead_MB;};
	/*
     * This method returns the quantity of virtual machines allocated at node
     */
    int getNumberOfVMs(){return vms.size();};

    /*
     * This method sets the input and output gates from vm to the network manager
     */
	virtual int setVM (cGate* oGateI, cGate* oGateO, cGate* iGateI, cGate* iGateO, int requestedMemory_KB, int uId, int pId);

    /*
     * This method delete the gates connected to the VM with id given as parameter
     */
    virtual void freeVM(int uId, int pId);

    /*
     * Virtual method to be implemented at scheduler.
     */
    virtual void schedulingMemory (Packet *) = 0;

protected:

    virtual int getVMGateIdx(int uId, int pId) = 0;

    /*
     * Process to migrate memory contents to other node
     */
	Ptr<icancloud_App_MEM_Message> migrationToMemoryContents(const Ptr<const icancloud_Migration_Message> &sm);

	/*
     * This method send the message to the proper gate
     */
	void sendMemoryMessage(Packet *msg);

};

} // namespace icancloud
} // namespace inet

#endif /* H_MEMORY_MANAGER_BASE_H_ */
