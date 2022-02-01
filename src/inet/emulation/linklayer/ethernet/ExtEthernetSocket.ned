//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.emulation.linklayer.ethernet;

//
// This module provides a bidirectional connection to an ethernet socket of the
// host computer which is running the simulation. It writes the packets arrived
// on 'upperLayerIn' gate to the specified real socket, and sends out packets
// arrived from the real socket on 'upperLayerOut' gate.
//
simple ExtEthernetSocket
{
    parameters:
        string device; // name of existing real ethernet device
        string namespace = default("");
        string packetNameFormat = default("Ext-%p-%t-%n"); // name of received packets
        @display("i=block/rxtx");
        @signal[packetSentToUpper](type=Packet);
        @signal[packetReceivedFromUpper](type=Packet);
        @signal[packetSent](type=Packet);
        @signal[packetReceived](type=Packet);
        @statistic[packetSentFromUpper](title="packets sent to higher layer"; source=packetSentToUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetReceivedFromUpper](title="packets received from higher layer"; source=packetReceivedFromUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input upperLayerIn;
        output upperLayerOut @loose;
}
