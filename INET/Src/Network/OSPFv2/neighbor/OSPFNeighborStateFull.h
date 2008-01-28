#ifndef __OSPFNEIGHBORSTATEFULL_HPP__
#define __OSPFNEIGHBORSTATEFULL_HPP__

#include "OSPFNeighborState.h"

namespace OSPF {

class NeighborStateFull : public NeighborState
{
public:
    virtual void ProcessEvent (Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState (void) const { return Neighbor::FullState; }
};

} // namespace OSPF

#endif // __OSPFNEIGHBORSTATEFULL_HPP__

