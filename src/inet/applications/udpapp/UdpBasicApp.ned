//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.udpapp;

import inet.applications.contract.IApp;

//
// Generates and sends UDP packets to specified destination addresses at configurable intervals.
// Supports both IPv4 and IPv6 addressing, with the ability to target multiple destinations
// by randomly selecting from a space-separated list of addresses for each transmission.
// The sending interval can be configured as a constant or random value (e.g., exponential
// distribution), making it suitable for modeling various traffic patterns and network load
// scenarios.
//
// While primarily designed as a traffic generator, the module also counts received packets,
// though it doesn't process their contents. It can be paired with various UDP receivers
// such as ~UdpSink, another ~UdpBasicApp, or ~UdpEchoApp. When used with ~UdpEchoApp,
// the module can measure round-trip times through the rcvdPkLifetime statistic.
//
// @see ~UdpSink, ~UdpEchoApp, ~UdpSocket
//
simple UdpBasicApp like IApp
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        int localPort = default(-1);  // Local port (-1: use ephemeral port)
        string destAddresses = default(""); // List of IP addresses, separated by spaces ("": don't send)
        string localAddress = default("");
        string packetName = default("UdpBasicAppData");
        int destPort;
        volatile int messageLength @unit(B); // Length of messages to generate, in bytes
        double startTime @unit(s) = default(this.sendInterval); // Application start time (start of the first packet)
        double stopTime @unit(s) = default(-1s);  // Time of finishing sending, -1s means forever
        volatile double sendInterval @unit(s); // Should usually be a random value, e.g. exponential(1)
        int timeToLive = default(-1); // If not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        bool dontFragment = default(false); // If true, asks IP to not fragment the message during routing
        int dscp = default(-1); // If not -1, set the DSCP field (on IPv4/IPv6) of sent packets to this value
        int tos = default(-1); // If not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        string multicastInterface = default("");  // If not empty, set the multicast output interface option on the socket (interface name expected)
        bool receiveBroadcast = default(false); // If true, makes the socket receive broadcast packets
        bool joinLocalMulticastGroups = default(false); // If true, makes the socket receive packets from all multicast groups set on local interfaces
        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[throughput](title="throughput"; unit=bps; source="throughput(packetReceived)"; record=vector);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkLifetime](title="received packet lifetime"; source="dataAge(packetReceived)"; unit=s; record=stats,vector; interpolationmode=none);
        @statistic[rcvdPkSeqNo](title="received packet sequence number"; source="appPkSeqNo(packetReceived)"; record=vector; interpolationmode=none);
    gates:
        input socketIn @labels(UdpCommand/up);
        output socketOut @labels(UdpCommand/down);
}
