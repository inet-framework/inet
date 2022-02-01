//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.emulation.transportlayer.udp;

import inet.transportlayer.contract.IUdp;

//
// This module provides UDP protocol services suitable for emulation.
// The lower part of the UDP protocol is realized in the real world using
// real UDP sockets of the host computer which is running the simulation.
//
simple ExtLowerUdp like IUdp
{
    parameters:
        string namespace = default("");
        string packetNameFormat = default("Ext-%p-%t-%n");
        @display("i=block/transport");
        @signal[packetSentToUpper](type=inet::Packet);
        @signal[packetReceivedFromUpper](type=inet::Packet);
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);

    gates:
        input appIn @labels(UdpControlInfo/down);
        input ipIn @labels(UdpHeader,Ipv4ControlInfo/up,Ipv6ControlInfo/up) @loose;
        output appOut @labels(UdpControlInfo/up);
        output ipOut @labels(UdpHeader,Ipv4ControlInfo/down,Ipv6ControlInfo/down) @loose;
}
