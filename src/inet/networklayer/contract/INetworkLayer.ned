//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.networklayer.contract;

//
// Interface for different network layers.
//
moduleinterface INetworkLayerUpper
{
    parameters:
        @display("i=block/fork");
    gates:
        input transportIn @labels(ITransportPacket/down);
        output transportOut @labels(ITransportPacket/up);
}

//
// Defines the interface between the network layer and link layer. Provides
// gates for sending and receiving packets to and from network interfaces.
// Implemented by various network layer modules like IPv4, IPv6, MPLS, etc.
//
moduleinterface INetworkLayerLower
{
    parameters:
        @display("i=block/fork");
    gates:
        input ifIn @labels(INetworkHeader/up);
        output ifOut @labels(INetworkHeader/down);
}

//
// Complete network layer interface that combines upper (transport) and lower
// (link) layer interfaces. Provides a unified interface for network protocols
// like IPv4, IPv6, MPLS, etc. Implementations typically include routing tables,
// address resolution protocols, and protocol-specific modules.
//
moduleinterface INetworkLayer extends INetworkLayerUpper, INetworkLayerLower
{
    parameters:
        @display("i=block/fork");
}
