//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_OSPFV2NEIGHBORSTATELOADING_H
#define __INET_OSPFV2NEIGHBORSTATELOADING_H

#include "inet/routing/ospfv2/neighbor/Ospfv2NeighborState.h"

namespace inet {

namespace ospfv2 {

class INET_API NeighborStateLoading : public NeighborState
{
  public:
    virtual void processEvent(Neighbor *neighbor, Neighbor::NeighborEventType event) override;
    virtual Neighbor::NeighborStateType getState() const override { return Neighbor::LOADING_STATE; }
};

} // namespace ospfv2

} // namespace inet

#endif

