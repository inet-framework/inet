//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/ospfv2/interface/Ospfv2InterfaceStateLoopback.h"

#include "inet/routing/ospfv2/interface/Ospfv2InterfaceStateDown.h"

namespace inet {
namespace ospfv2 {

void InterfaceStateLoopback::processEvent(Ospfv2Interface *intf, Ospfv2Interface::Ospfv2InterfaceEventType event)
{
    if (event == Ospfv2Interface::INTERFACE_DOWN) {
        intf->reset();
        changeState(intf, new InterfaceStateDown, this);
    }
    else if (event == Ospfv2Interface::UNLOOP_INDICATION) {
        changeState(intf, new InterfaceStateDown, this);
    }
}

} // namespace ospfv2
} // namespace inet

