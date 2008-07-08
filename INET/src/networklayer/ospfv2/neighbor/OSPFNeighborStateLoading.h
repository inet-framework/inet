#ifndef __INET_OSPFNEIGHBORSTATELOADING_H
#define __INET_OSPFNEIGHBORSTATELOADING_H

#include "OSPFNeighborState.h"

namespace OSPF {

class NeighborStateLoading : public NeighborState
{
public:
    virtual void ProcessEvent(Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState(void) const { return Neighbor::LoadingState; }
};

} // namespace OSPF

#endif // __INET_OSPFNEIGHBORSTATELOADING_H

