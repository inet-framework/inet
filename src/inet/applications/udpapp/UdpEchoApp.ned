//
// Copyright (C) 2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.udpapp;

import inet.applications.contract.IApp;

//
// Listens on a UDP port and sends back each received packet to its sender.
// Note: when used together with ~UdpBasicApp, ~UdpBasicApp's "received packet
// lifetime" statistic will record round-trip times.
//
simple UdpEchoApp like IApp
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        int localPort;  // Local port to listen on
        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
        @signal[packetSent](type=inet::Packet);
        @statistic[echoedPk](title="packets echoed"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input socketIn @labels(UdpCommand/up);
        output socketOut @labels(UdpCommand/down);
}

