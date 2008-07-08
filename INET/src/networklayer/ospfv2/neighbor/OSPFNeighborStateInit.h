#ifndef __INET_OSPFNEIGHBORSTATEINIT_H
#define __INET_OSPFNEIGHBORSTATEINIT_H

#include "OSPFNeighborState.h"

namespace OSPF {

class NeighborStateInit : public NeighborState
{
public:
    virtual void ProcessEvent(Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState(void) const { return Neighbor::InitState; }
};

} // namespace OSPF

#endif // __INET_OSPFNEIGHBORSTATEINIT_H

