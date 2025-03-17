//
// Copyright (C) 2001 Monash University, Australia
// Copyright (C) 2012-2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.pingapp;

import inet.applications.contract.IApp;

//
// Generates ping requests and calculates packet loss and round trip times of the replies.
// Functions similarly to the standard ping utility but can target multiple destinations
// simultaneously. Destinations can be specified as a space-separated list of IP addresses
// or module names, with '*' representing all network interfaces in the simulation.
// This is particularly useful for verifying network connectivity and proper routing
// table configuration.
//
// Ping requests are sent at configurable intervals to each destination. The count
// parameter controls how many pings are sent to each destination before moving to
// the next one, with an optional sleep period between destinations. The application
// tracks sequence numbers to detect lost packets, out-of-order arrivals, and
// calculates detailed statistics including packet loss rate and round-trip times.
//
// The module provides extensive statistics via signals, including round-trip time,
// packet loss count, and out-of-order arrival count. These metrics can be used
// to evaluate network performance and reliability.
//
// @see ~Icmp, ~Icmpv6, ~EchoPacket
//
simple PingApp like IApp
{
    parameters:
        string destAddr = default(""); // Destination address(es), separated by spaces, "*" means all IPv4/IPv6 interfaces in the entire simulation
        string srcAddr = default(""); // Source address (useful with multi-homing)
        int packetSize @unit(B) = default(56B); // Size of ping payload, in bytes
        volatile double sendInterval @unit(s) = default(1s); // Time to wait between pings (can be random)
        int hopLimit = default(-1); // TTL or hopLimit for IP packets
        int count = default(-1); // Number of ping requests sent to a single destination address, -1 means continuously (only first address will be used from destAddr)
        double startTime @unit(s) = default(uniform(0s, this.sleepDuration + this.sendInterval)); // Send the first ping at startTime
        double stopTime @unit(s) = default(-1s); // Time to finish sending, negative values mean forever
        volatile double sleepDuration @unit(s) = default(0s); // Time spent in sleep between switching destinations
        bool continuous = default(false);  // Whether to continuously ping the destinations in a round-robin fashion
        bool printPing = default(false); // Log to stdout
        string crcMode @enum("declared", "computed") = default("declared");
        string networkProtocol = default(""); // Uses the specified network protocol for communication; uses the address-specified default network protocol when the parameter is empty
        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after the lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for the lifecycle stop operation
        @signal[rtt](type=simtime_t);
        @signal[numLost](type=long);
        @signal[numOutOfOrderArrivals](type=long);
        @signal[pingTxSeq](type=long);
        @signal[pingRxSeq](type=long);
        @statistic[rtt](title="ping round-trip time"; unit=s; record=histogram,vector; interpolationmode=none);
        @statistic[numLost](title="pings lost"; record=last,vector; interpolationmode=none);
        @statistic[numOutOfOrderArrivals](title="ping out-of-order arrivals"; record=last,vector; interpolationmode=none);
        @statistic[pingTxSeq](title="ping tx seq"; record=count,vector; interpolationmode=none);
        @statistic[pingRxSeq](title="ping rx seq"; record=count,vector; interpolationmode=none);
        @selfMessageKinds(inet::PingSelfKinds);    // Kind used in self messages
    gates:
        input socketIn @labels(ITransportPacket/up);
        output socketOut @labels(ITransportPacket/down);
}
