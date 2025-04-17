//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.udpapp;

import inet.common.SimpleModule;

//
// Provides UDP socket handling for generic applications.
//
simple UdpSocketIo extends SimpleModule
{
    parameters:
        @class(UdpSocketIo);
        string interfaceTableModule;
        string localAddress = default("");
        int localPort = default(-1);
        string destAddress = default("");
        int destPort;
        int timeToLive = default(-1); // If not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        bool dontFragment = default(false); // If true, ask IP to not fragment the message during routing
        int dscp = default(-1); // If not -1, set the DSCP (IPv4/IPv6) field of sent packets to this value
        int tos = default(-1); // If not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        string multicastInterface = default("");  // If not empty, set the multicast output interface option on the socket (interface name expected)
        object multicastAddresses = default([]); // List of multicast addresses that the application socket joins
        bool receiveBroadcast = default(false); // If true, make the socket receive broadcast packets
        bool joinLocalMulticastGroups = default(false); // If true, make the socket receive packets from all multicast groups set on local interfaces
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finishes
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
