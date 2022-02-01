//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_OSPFV2NEIGHBORSTATE_H
#define __INET_OSPFV2NEIGHBORSTATE_H

#include "inet/routing/ospfv2/neighbor/Ospfv2Neighbor.h"

namespace inet {

namespace ospfv2 {

class INET_API NeighborState
{
  protected:
    void changeState(Neighbor *neighbor, NeighborState *newState, NeighborState *currentState);
    bool updateLsa(Neighbor *neighbor);

  public:
    virtual ~NeighborState() {}

    virtual void processEvent(Neighbor *neighbor, Neighbor::NeighborEventType event) = 0;
    virtual Neighbor::NeighborStateType getState() const = 0;
};

} // namespace ospfv2

} // namespace inet

#endif

