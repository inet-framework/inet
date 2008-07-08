#ifndef __INET_OSPFNEIGHBORSTATEEXCHANGE_H
#define __INET_OSPFNEIGHBORSTATEEXCHANGE_H

#include "OSPFNeighborState.h"

namespace OSPF {

class NeighborStateExchange : public NeighborState
{
public:
    virtual void ProcessEvent(Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState(void) const { return Neighbor::ExchangeState; }
};

} // namespace OSPF

#endif // __INET_OSPFNEIGHBORSTATEEXCHANGE_H

