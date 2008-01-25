#ifndef __OSPFNEIGHBORSTATELOADING_HPP__
#define __OSPFNEIGHBORSTATELOADING_HPP__

#include "OSPFNeighborState.h"

namespace OSPF {

class NeighborStateLoading : public NeighborState
{
public:
    virtual void ProcessEvent (Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState (void) const { return Neighbor::LoadingState; }
};

} // namespace OSPF

#endif // __OSPFNEIGHBORSTATELOADING_HPP__

