#ifndef __INET_OSPFV3NEIGHBORSTATEFULL_H_
#define __INET_OSPFV3NEIGHBORSTATEFULL_H_

#include "inet/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborState.h"
#include "inet/common/INETDefs.h"

namespace inet{

class INET_API OSPFv3NeighborStateFull : public OSPFv3NeighborState
{
    /*
     * Full adjacency, they will appear in router and network LSAs.
     */
  public:
    void processEvent(OSPFv3Neighbor* neighbor, OSPFv3Neighbor::OSPFv3NeighborEventType event) override;
    virtual OSPFv3Neighbor::OSPFv3NeighborStateType getState() const override { return OSPFv3Neighbor::FULL_STATE; }
    std::string getNeighborStateString(){return std::string("OSPFv3NeighborStateFull");};
    ~OSPFv3NeighborStateFull(){};
};

}//namespace inet
#endif
