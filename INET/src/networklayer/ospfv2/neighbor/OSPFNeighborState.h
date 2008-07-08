#ifndef __INET_OSPFNEIGHBORSTATE_H
#define __INET_OSPFNEIGHBORSTATE_H

#include "OSPFNeighbor.h"

namespace OSPF {

class NeighborState {
protected:
    void ChangeState(Neighbor* neighbor, NeighborState* newState, NeighborState* currentState);

public:
    virtual ~NeighborState() {}

    virtual void ProcessEvent(Neighbor* neighbor, Neighbor::NeighborEventType event) = 0;
    virtual Neighbor::NeighborStateType GetState(void) const = 0;
};

} // namespace OSPF

#endif // __INET_OSPFNEIGHBORSTATE_H

