//
// Copyright (C) 2015 Irene Ruengeler
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

package inet.applications.packetdrill;

import inet.applications.contract.IApp;

//
// This is a port of Google's Packetdrill testing tool into the INET Framework.
//
// Google released the script-based Packetdrill tool in 2013. It allows one
// to test transport protocols like UDP and TCP on Linux and BSD-based
// operating systems. Scripts define test cases that inject packets to the
// protocol implementation under test, perform operations using the API
// controlling the transport protocol, and verify the sending of packets, all
// at specified times. The ported version provides a simple and powerful
// method of testing transport protocols implemented in INET.
//
simple PacketDrillApp like IApp
{
    parameters:
        string scriptFile;
        double startTime @unit(s) = default(0s);
        int localPort; // port number of the initiator
        int remotePort;
        string localAddress;
        string remoteAddress;
        string interfaceTableModule;
        string interface;   // tun interface name
        int inboundStreams = default(20);
        int outboundStreams = default(20);
        int streamReset = default(0);
        string crcMode @enum("declared", "computed") = default("declared");
        double latency @unit(s) = default(0s);
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
}

