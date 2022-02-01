//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_OSPFV2INTERFACESTATE_H
#define __INET_OSPFV2INTERFACESTATE_H

#include "inet/routing/ospfv2/interface/Ospfv2Interface.h"

namespace inet {

namespace ospfv2 {

class INET_API Ospfv2InterfaceState
{
  protected:
    void changeState(Ospfv2Interface *intf, Ospfv2InterfaceState *newState, Ospfv2InterfaceState *currentState);
    void calculateDesignatedRouter(Ospfv2Interface *intf);
    void printElectionResult(const Ospfv2Interface *onInterface, DesignatedRouterId DR, DesignatedRouterId BDR);

  public:
    virtual ~Ospfv2InterfaceState() {}

    virtual void processEvent(Ospfv2Interface *intf, Ospfv2Interface::Ospfv2InterfaceEventType event) = 0;
    virtual Ospfv2Interface::Ospfv2InterfaceStateType getState() const = 0;
};

} // namespace ospfv2

} // namespace inet

#endif

