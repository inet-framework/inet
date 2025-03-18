//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ethernet.common;

import inet.common.SimpleModule;

//
// Determines whether a received packet should be delivered locally or forwarded
// based on its destination MAC address. Packets addressed to the local interface
// are pushed to the deliveryOut gate, while packets addressed to other nodes are
// pushed to the forwardingOut gate. Handles unicast, broadcast, and multicast
// addresses appropriately. Note that broadcast packets are only pushed to the
// forwardingOut gate (TODO why?), and multicast packets are pushed to the
// either the  deliveryOut gate (if matching) or the forwardingOut gate, but not
// both (TODO why?).
//
simple LocalDelivery extends SimpleModule
{
    parameters:
        @class(LocalDelivery);
        string interfaceTableModule;
    gates:
        input in;
        output deliveryOut;
        output forwardingOut;
}

