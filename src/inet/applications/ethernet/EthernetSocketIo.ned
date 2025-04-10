//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ethernet;

import inet.common.SimpleModule;

//
// Provides Ethernet socket handling for generic applications.
//
simple EthernetSocketIo extends SimpleModule
{
    parameters:
        @class(EthernetSocketIo);
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
