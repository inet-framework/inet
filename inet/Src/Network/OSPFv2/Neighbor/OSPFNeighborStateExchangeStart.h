#ifndef __OSPFNEIGHBORSTATEEXCHANGESTART_HPP__
#define __OSPFNEIGHBORSTATEEXCHANGESTART_HPP__

#include "OSPFNeighborState.h"

namespace OSPF {

class NeighborStateExchangeStart : public NeighborState
{
public:
    virtual void ProcessEvent (Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState (void) const { return Neighbor::ExchangeStartState; }
};

} // namespace OSPF

#endif // __OSPFNEIGHBORSTATEEXCHANGESTART_HPP__

