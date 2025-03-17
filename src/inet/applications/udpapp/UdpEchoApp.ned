//
// Copyright (C) 2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.udpapp;

import inet.applications.contract.IApp;

//
// Implements a simple UDP echo service that listens on a specified port and echoes back
// any received packets to their original sender without modifying their contents. This
// application provides a straightforward way to test network connectivity, measure
// round-trip times, and evaluate basic UDP communication in simulated networks.
//
// When paired with clients like ~UdpBasicApp, it enables round-trip time measurements
// through the client's "received packet lifetime" statistics. The application maintains
// a count of echoed packets and can join multicast groups defined in the interface table,
// making it suitable for both unicast and multicast testing scenarios.
//
// @see ~UdpBasicApp, ~UdpSocket, ~UdpSink
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
