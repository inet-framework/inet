#ifndef __OSPFNEIGHBORSTATEEXCHANGE_HPP__
#define __OSPFNEIGHBORSTATEEXCHANGE_HPP__

#include "OSPFNeighborState.h"

namespace OSPF {

class NeighborStateExchange : public NeighborState
{
public:
    virtual void ProcessEvent (Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState (void) const { return Neighbor::ExchangeState; }
};

} // namespace OSPF

#endif // __OSPFNEIGHBORSTATEEXCHANGE_HPP__

