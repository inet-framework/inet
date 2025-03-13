//
// Copyright (C) 2015 Irene Ruengeler
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.tun;

//
// Implements a TUN (network tunnel) interface that allows applications to
// send and receive packets directly to/from the network layer, bypassing
// the normal protocol stack. This module enables the creation of virtual
// network interfaces that can be used for tunneling, VPNs, and other
// network virtualization applications.
//
// Applications can connect to this module using TunSocket to send and
// receive packets. The module maintains a list of connected sockets and
// routes packets between the network layer and the appropriate applications.
//
// @see ~TunInterface, ~ITunnelInterface, ~TunSocket
//
simple Tun
{
    parameters:
        string interfaceTableModule;    // The path to the InterfaceTable module
        int mtu @unit(B) = default(1500B);
        @display("i=block/tunnel");

        @signal[packetSentToLower](type=cPacket);
        @signal[packetReceivedFromLower](type=cPacket);
        @signal[packetSentToUpper](type=cPacket);
        @signal[packetReceivedFromUpper](type=cPacket);

    gates:
        input upperLayerIn @messageKinds(inet::TunCommandCode);    // From upper layers
        output upperLayerOut @messageKinds(inet::TunSocketIndication);  // To upper layers
}
