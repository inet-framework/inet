//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ethernet.common;

//
// Determines whether a received packet should be delivered locally or forwarded
// based on its destination MAC address. Packets addressed to the local interface
// are sent to the deliveryOut gate, while packets addressed to other nodes are
// sent to the forwardingOut gate. Handles unicast, broadcast, and multicast
// addresses appropriately.
//
simple LocalDelivery
{
    parameters:
        string interfaceTableModule;
    gates:
        input in;
        output deliveryOut;
        output forwardingOut;
}
