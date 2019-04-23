//
// Class that defines the behavior of hypervisor network controller.
//
// The scheduling policy of this module is first input first output.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//

#ifndef HNETSCHEDFIFO_H_
#define HNETSCHEDFIFO_H_

#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_NetworkManager/Managers/H_NETManager_Base.h"

namespace inet {

namespace icancloud {


class H_NET_SCHED_FIFO : public H_NETManager_Base{

protected:

public:

    virtual ~H_NET_SCHED_FIFO();

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
    void schedulingNET(Packet *) override;

    /**
    * Process a response message.
    * @param sm Request message.
    */
    void processResponseMessage(Packet  *) override;
};

} // namespace icancloud
} // namespace inet

#endif /* HCPUSCHEDFIFO_H_ */
