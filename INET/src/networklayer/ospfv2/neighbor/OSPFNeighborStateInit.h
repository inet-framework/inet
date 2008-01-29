#ifndef __OSPFNEIGHBORSTATEINIT_HPP__
#define __OSPFNEIGHBORSTATEINIT_HPP__

#include "OSPFNeighborState.h"

namespace OSPF {

class NeighborStateInit : public NeighborState
{
public:
    virtual void ProcessEvent (Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState (void) const { return Neighbor::InitState; }
};

} // namespace OSPF

#endif // __OSPFNEIGHBORSTATEINIT_HPP__

