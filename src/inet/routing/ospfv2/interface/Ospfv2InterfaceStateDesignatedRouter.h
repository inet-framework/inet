//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_OSPFV2INTERFACESTATEDESIGNATEDROUTER_H
#define __INET_OSPFV2INTERFACESTATEDESIGNATEDROUTER_H

#include "inet/routing/ospfv2/interface/Ospfv2InterfaceState.h"

namespace inet {

namespace ospfv2 {

class INET_API InterfaceStateDesignatedRouter : public Ospfv2InterfaceState
{
  public:
    virtual void processEvent(Ospfv2Interface *intf, Ospfv2Interface::Ospfv2InterfaceEventType event) override;
    virtual Ospfv2Interface::Ospfv2InterfaceStateType getState() const override { return Ospfv2Interface::DESIGNATED_ROUTER_STATE; }
};

} // namespace ospfv2

} // namespace inet

#endif

