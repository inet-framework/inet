//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.emulation.networklayer.ipv4;

//
// This module provides a bidirectional connection to a real TUN device of the
// host computer which is running the simulation. It writes the packets arrived
// on 'lowerLayerIn' gate to the specified real TUN device, and sends out packets
// arrived from real TAP device on 'lowerLayerOut' gate.
//
simple ExtIpv4TunDevice
{
    parameters:
        string device;
        string namespace = default("");
        string packetNameFormat = default("Ext");
        @signal[packetSentToLower](type=Packet);
        @signal[packetReceivedFromLower](type=Packet);
        @signal[packetSent](type=Packet);
        @signal[packetReceived](type=Packet);
        @statistic[packetSentFromLower](title="packets sent to higher layer"; source=packetSentToLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetReceivedFromLower](title="packets received from higher layer"; source=packetReceivedFromLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input lowerLayerIn @labels(EtherFrame);
        output lowerLayerOut @labels(EtherFrame);
}

