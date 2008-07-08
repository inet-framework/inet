#ifndef __INET_OSPFNEIGHBORSTATEDOWN_H
#define __INET_OSPFNEIGHBORSTATEDOWN_H

#include "OSPFNeighborState.h"

namespace OSPF {

class NeighborStateDown : public NeighborState
{
public:
    virtual void ProcessEvent(Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState(void) const { return Neighbor::DownState; }
};

} // namespace OSPF

#endif // __INET_OSPFNEIGHBORSTATEDOWN_H

