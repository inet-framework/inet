//
// Copyright (C) 2015 Irene Ruengeler
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.tun;

import inet.common.SimpleModule;

//
// Implements a TUN (network tunnel) interface that provides a virtual network
// interface for applications. This is the main component of ~TunInterface.
// Allows applications to send and receive packets
// directly to/from the network layer using a socket-like API. Maintains a list
// of registered sockets and handles commands for opening, closing, and using
// the TUN interface. Processes packets between upper layers and the network
// layer, adding appropriate tags for routing.
//
simple Tun extends SimpleModule
{
    parameters:
        @class(Tun);
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
