//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.loopback;

//
// Implements a loopback interface that immediately sends back any received packet
// to the upper layer. Packets are not altered in any way. This module is used to
// support the loopback interface in network nodes, allowing internal communication
// between protocols within the same node.
//
// @see ~LoopbackInterface, ~ILoopbackInterface
//
simple Loopback
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        int mtu @unit(B) = default(4470B);
        @display("i=block/rxtx");

        @signal[packetSentToUpper](type=cPacket);
        @signal[packetReceivedFromUpper](type=cPacket);
        @statistic[rcvdPkFromHl](title="packets received from higher layer"; source=packetReceivedFromUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[passedUpPk](title="packets passed to higher layer"; source=packetSentToUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);

    gates:
        input upperLayerIn;
        output upperLayerOut;
}
