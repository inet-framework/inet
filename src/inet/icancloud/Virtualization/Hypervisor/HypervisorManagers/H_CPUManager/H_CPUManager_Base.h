//
// Class abstract defining the base of the CPU controller.
//
// The virtual methods to be implemented at cpu manager concrete policy are:
//      - virtual void schedulingCPU(icancloud_Message *msg)
//      - virtual void processHardwareResponse(icancloud_Message *msg)
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//

#ifndef H_CPU_MANAGER_BASE_H_
#define H_CPU_MANAGER_BASE_H_

#include "inet/icancloud/Base/icancloud_Base.h"

namespace inet {

namespace icancloud {


class H_CPUManager_Base: public icancloud_Base {

protected:

	/** Number of CPUs */
	unsigned int numCPUs;

	// Gate for the node
	int nodeGate;

    /** Output gate to Service Redirector. */
    cGate** toNodeCPU;

    /** Input gate from Service Redirector. */
    cGate** fromNodeCPU;

    /** Output gates to CPU. */
    cGateManager* toVMCPU;

    /** Input gates from CPU. */
    cGateManager* fromVMCPU;

    /*
     * Structure to control the vms linked
     */
    struct vmControl_t{
        vector<int> gates;
        int uId;
        int pId;
    };

    typedef vmControl_t vmControl;

    vector<vmControl*> vms;

    /*
     * Structure for the overhead
     */
    double computing_overhead;

    struct overhead_t{
        Packet* msg;
    };

    typedef overhead_t overhead;

    vector<overhead*> overheadStructure;

protected:

    /*
     * Destructor
     */
    virtual ~H_CPUManager_Base();

    /*
     * Module initialization
     */
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    
    /*
     * Module finalization
     */
    virtual void finish() override;

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

    /**
    * Process a request to the core message.(To be used by scheduler)
    * @param sm Request message.
    */
    void sendRequestToCore(Packet *, cGate* gate, int gateIdx);

    /*
     * Process self message
     */
    virtual void processSelfMessage (cMessage *msg) override;;

    /*
     * Get out gate
     */
    cGate* getOutGate (cMessage *msg) override;

    /*
     * Virtual method to be implemented at scheduler.
     */
    virtual void schedulingCPU(Packet *) = 0;

    /*
     * Virtual method to be implemented at scheduler cpu.
     */
    virtual void processHardwareResponse(Packet *) = 0;

public:
    /*
     * This method connect the gates (oGates) output and (iGates) input
     * to the cpu manager
     */
    void setVM(cGate** oGates, cGate** iGates, int numCores, int uId, int pId);

    /*
     * This method delete the gates connected to the VM with id given as parameter
     */
    void freeVM(int uId, int pId);


    /*
     * This method returns the quantity of virtual machines allocated at node
     */
    int getNumberOfVMs(){return vms.size();};


};

} // namespace icancloud
} // namespace inet

#endif /* H_CPU_MANAGER_BASE_H_ */
