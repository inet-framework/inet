#ifndef __INET_OSPFV3NEIGHBORSTATEEXSTART_H_
#define __INET_OSPFV3NEIGHBORSTATEEXSTART_H_

#include "inet/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborState.h"
#include "inet/common/INETDefs.h"

namespace inet{

class INET_API OSPFv3NeighborStateExStart : public OSPFv3NeighborState
{
    /*
     * First step in creating adjacency. A decision is made which of the two routers will be the master
     * and which will be the slave. Initial DD sequence number is decided. Neighbor conversations in
     * this or higher states are called adjacencies.
     */
  public:
    void processEvent(OSPFv3Neighbor* neighbor, OSPFv3Neighbor::OSPFv3NeighborEventType event) override;
    virtual OSPFv3Neighbor::OSPFv3NeighborStateType getState() const override { return OSPFv3Neighbor::EXCHANGE_START_STATE; }
    std::string getNeighborStateString(){return std::string("OSPFv3NeighborStateExStart");};
    ~OSPFv3NeighborStateExStart(){};
};

}//namespace inet
#endif
