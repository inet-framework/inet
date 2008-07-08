#ifndef __INET_OSPFNEIGHBORSTATEATTEMPT_H
#define __INET_OSPFNEIGHBORSTATEATTEMPT_H

#include "OSPFNeighborState.h"

namespace OSPF {

class NeighborStateAttempt : public NeighborState
{
public:
    virtual void ProcessEvent(Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState(void) const { return Neighbor::AttemptState; }
};

} // namespace OSPF

#endif // __INET_OSPFNEIGHBORSTATEATTEMPT_H

