#ifndef __INET_OSPFV3NEIGHBORSTATE2WAY_H_
#define __INET_OSPFV3NEIGHBORSTATE2WAY_H_

#include "inet/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborState.h"
#include "inet/common/INETDefs.h"

namespace inet{

class INET_API OSPFv3NeighborState2Way : public OSPFv3NeighborState
{
    /*
     * Bidirectional communication established. Highest state before exchanging databases.
     * DR and BDR are chosen during this state.
     */
  public:
    void processEvent(OSPFv3Neighbor* neighbor, OSPFv3Neighbor::OSPFv3NeighborEventType event) override;
    virtual OSPFv3Neighbor::OSPFv3NeighborStateType getState() const override { return OSPFv3Neighbor::TWOWAY_STATE; }
    std::string getNeighborStateString(){return std::string("OSPFv3NeighborState2Way");};
    ~OSPFv3NeighborState2Way(){};


};

}//namespace inet
#endif
