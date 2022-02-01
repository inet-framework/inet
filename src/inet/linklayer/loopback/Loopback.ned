//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.loopback;

//
// Loopback interface module implementation.
//
// @see ~LoopbackInterface
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

