#ifndef __INET_OSPFV3NEIGHBORSTATEFULL_H_
#define __INET_OSPFV3NEIGHBORSTATEFULL_H_

#include "inet/routing/ospf/v3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospf/v3/neighbor/Ospfv3NeighborState.h"
#include "inet/common/INETDefs.h"

namespace inet{

class INET_API Ospfv3NeighborStateFull : public Ospfv3NeighborState
{
    /*
     * Full adjacency, they will appear in router and network LSAs.
     */
  public:
    virtual void processEvent(Ospfv3Neighbor* neighbor, Ospfv3Neighbor::Ospfv3NeighborEventType event) override;
    virtual Ospfv3Neighbor::Ospfv3NeighborStateType getState() const override { return Ospfv3Neighbor::FULL_STATE; }
    virtual std::string getNeighborStateString() override { return std::string("Ospfv3NeighborStateFull"); }
    ~Ospfv3NeighborStateFull(){};
};

}//namespace inet

#endif

