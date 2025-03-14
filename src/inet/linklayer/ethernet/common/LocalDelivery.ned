//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

//
// Packet processing module that determines whether incoming frames should be delivered locally or forwarded.
// Examines MAC addresses to decide the appropriate path for each packet. Delivers packets to the local 
// protocol stack if they are addressed to the local interface (unicast to interface MAC, multicast that 
// the interface subscribes to), otherwise forwards them to other network nodes.
//
// @see ~MacAddressTag, ~InterfaceTag, ~IInterfaceTable
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
