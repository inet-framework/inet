//
// Class that defines the behavior of hypervisor cpu controller.
//
// The scheduling policy of this module is first input first output.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//


#ifndef HCPUSCHEDFIFO_H_
#define HCPUSCHEDFIFO_H_

#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_CPUManager/H_CPUManager_Base.h"
#include <deque>

namespace inet {

namespace icancloud {


class H_CPU_SCHED_FIFO : public H_CPUManager_Base{

protected:

    /** Array to show the ID of VM executing in the core 'i'*/
    vector<int> vmIDs;

    /** Queue of messages trying to access to a core */
    std::deque<Packet*> coreQueue;

public:

    virtual ~H_CPU_SCHED_FIFO();

    /*
     * Module initialization
     */
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    

    /*
     * Module finalization
     */
    void finish() override;

    /*
     * Scheduler fifo
     */
    void schedulingCPU(Packet *msg) override;

    /**
    * Process a response message.
    * @param sm Request message.
    */
    void processHardwareResponse(Packet *) override;
};

} // namespace icancloud
} // namespace inet

#endif /* HCPUSCHEDFIFO_H_ */
