//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.transportlayer.tcp_common;

import inet.transportlayer.contract.ITcp;

//
// Sends fabricated TCP packets. This is a base implementation,
// you'll probably have to customize it in C++ according to
// what you want to send and when you want to send it.
//
// Compatible with both IPv4 (~Ipv4) and IPv6 (~Ipv6).
//
simple TcpSpoof like ITcp
{
    parameters:
        string srcAddress = default("");  // local address; may be left empty ("")
        string destAddress = default(""); // destination address
        int srcPort; // local port number
        int destPort; // destination port number
        int seqNo; // sequence number (-1 for TCP ISS)
        bool isSYN = default(true); // whether to set SYN bit on packet
        double t @unit(s); // simulation time to send at
        @display("i=block/square");
        @signal[packetSent](type=cPacket);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);

    gates:
        input appIn @labels(TcpCommand/down); // Not used!
        output appOut @labels(TcpCommand/up); // Not used!
        output ipOut @labels(TcpHeader,Ipv4ControlInfo/down,Ipv6ControlInfo/down);
        input ipIn @labels(TcpHeader,Ipv4ControlInfo/up,Ipv6ControlInfo/up); // Not used!
}

