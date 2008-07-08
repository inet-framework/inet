#ifndef __INET_OSPFNEIGHBORSTATEFULL_H
#define __INET_OSPFNEIGHBORSTATEFULL_H

#include "OSPFNeighborState.h"

namespace OSPF {

class NeighborStateFull : public NeighborState
{
public:
    virtual void ProcessEvent(Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState(void) const { return Neighbor::FullState; }
};

} // namespace OSPF

#endif // __INET_OSPFNEIGHBORSTATEFULL_H

