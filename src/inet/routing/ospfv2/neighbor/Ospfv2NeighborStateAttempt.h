//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_OSPFV2NEIGHBORSTATEATTEMPT_H
#define __INET_OSPFV2NEIGHBORSTATEATTEMPT_H

#include "inet/routing/ospfv2/neighbor/Ospfv2NeighborState.h"

namespace inet {

namespace ospfv2 {

class INET_API NeighborStateAttempt : public NeighborState
{
  public:
    virtual void processEvent(Neighbor *neighbor, Neighbor::NeighborEventType event) override;
    virtual Neighbor::NeighborStateType getState() const override { return Neighbor::ATTEMPT_STATE; }
};

} // namespace ospfv2

} // namespace inet

#endif

