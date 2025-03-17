//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ethernet;

//
// Provides a bridge between application-level traffic and Ethernet protocol,
// handling the details of socket management and packet transmission. Acts as an
// adapter that converts between application data and Ethernet socket commands,
// simplifying the development of data link layer applications by abstracting
// away the complexities of Ethernet socket operations.
//
// Offers configuration options for Ethernet communication including interface
// selection, local and remote MAC addresses, and protocol type. The module
// maintains statistics on sent and received packets, enabling detailed analysis
// of traffic patterns. Designed to be used as a component within modular Ethernet
// applications, particularly those built with the queueing model architecture.
// Supports lifecycle operations for controlled startup and shutdown in simulation
// scenarios.
//
// @see ~EthernetApp, ~EthernetSourceApp, ~EthernetSinkApp, ~EthernetSocket
//
simple EthernetSocketIo
{
    parameters:
        string interfaceTableModule; // Relative module path of the interface table
        string interface = default(""); // Network interface name
        string protocol = default(""); // Bind the socket to this protocol if specified
        string localAddress = default(""); // Local Ethernet MAC address
        string remoteAddress = default(""); // Remote Ethernet MAC address
        bool steal = default(false); // Steal packets
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
        @lifecycleSupport();
        @display("i=block/socket");
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input trafficIn;
        output trafficOut;
        input socketIn;
        output socketOut;
}
