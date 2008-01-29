#ifndef __OSPFNEIGHBORSTATEATTEMPT_HPP__
#define __OSPFNEIGHBORSTATEATTEMPT_HPP__

#include "OSPFNeighborState.h"

namespace OSPF {

class NeighborStateAttempt : public NeighborState
{
public:
    virtual void ProcessEvent (Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState (void) const { return Neighbor::AttemptState; }
};

} // namespace OSPF

#endif // __OSPFNEIGHBORSTATEATTEMPT_HPP__

