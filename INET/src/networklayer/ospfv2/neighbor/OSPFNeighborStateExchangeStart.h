#ifndef __INET_OSPFNEIGHBORSTATEEXCHANGESTART_H
#define __INET_OSPFNEIGHBORSTATEEXCHANGESTART_H

#include "OSPFNeighborState.h"

namespace OSPF {

class NeighborStateExchangeStart : public NeighborState
{
public:
    virtual void ProcessEvent(Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState(void) const { return Neighbor::ExchangeStartState; }
};

} // namespace OSPF

#endif // __INET_OSPFNEIGHBORSTATEEXCHANGESTART_H

