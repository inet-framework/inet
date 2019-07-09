#ifndef __INET_OSPFV3NEIGHBORSTATEATTEMPT_H_
#define __INET_OSPFV3NEIGHBORSTATEATTEMPT_H_

#include "inet/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborState.h"
#include "inet/common/INETDefs.h"

namespace inet{

class INET_API OSPFv3NeighborStateAttempt : public OSPFv3NeighborState
{
    /*
     * Only for NBMA networks. No information received from neighbor but more concerted effort
     * should be made to establish connection. This is done by sending hello in hello intervals.
     */
  public:
    void processEvent(OSPFv3Neighbor* neighbor, OSPFv3Neighbor::OSPFv3NeighborEventType event) override;
    virtual OSPFv3Neighbor::OSPFv3NeighborStateType getState() const override { return OSPFv3Neighbor::ATTEMPT_STATE; }
    std::string getNeighborStateString(){return std::string("OSPFv3NeighborStateAttempt");};
    ~OSPFv3NeighborStateAttempt(){};

};

}//namespace inet
#endif
