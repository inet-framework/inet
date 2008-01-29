#ifndef __OSPFNEIGHBORSTATETWOWAY_HPP__
#define __OSPFNEIGHBORSTATETWOWAY_HPP__

#include "OSPFNeighborState.h"

namespace OSPF {

class NeighborStateTwoWay : public NeighborState
{
public:
    virtual void ProcessEvent (Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState (void) const { return Neighbor::TwoWayState; }
};

} // namespace OSPF

#endif // __OSPFNEIGHBORSTATETWOWAY_HPP__

