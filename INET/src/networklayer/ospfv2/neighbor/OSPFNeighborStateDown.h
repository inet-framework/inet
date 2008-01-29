#ifndef __OSPFNEIGHBORSTATEDOWN_HPP__
#define __OSPFNEIGHBORSTATEDOWN_HPP__

#include "OSPFNeighborState.h"

namespace OSPF {

class NeighborStateDown : public NeighborState
{
public:
    virtual void ProcessEvent (Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState (void) const { return Neighbor::DownState; }
};

} // namespace OSPF

#endif // __OSPFNEIGHBORSTATEDOWN_HPP__

