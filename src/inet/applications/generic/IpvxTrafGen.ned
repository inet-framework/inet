//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.generic;

//
// Generates configurable traffic directly over IPv4 or IPv6, bypassing transport
// protocols like TCP and UDP. Sends datagrams to specified destination addresses
// at regular or stochastically determined intervals. When multiple destination
// addresses are configured, randomly selects one for each packet, enabling various
// traffic distribution patterns.
//
// Provides extensive configuration options including start/stop times, packet size,
// sending interval, and protocol field values. The module can be configured to send
// a specific number of packets or to operate continuously until simulation end.
// Supports both deterministic and random traffic patterns through the volatile
// parameters, making it suitable for various network testing scenarios.
//
// Can communicate with ~IpvxTrafSink modules or other ~IpvxTrafGen instances, as
// it also includes packet reception capabilities. Collects statistics on sent and
// received packets, including counts, sizes, and end-to-end delays.
//
// @see ~IpvxTrafSink, ~IIpvxTrafficGenerator
//
simple IpvxTrafGen like IIpvxTrafficGenerator
{
    parameters:
        double startTime @unit(s) = default(1s); // Time of sending the first packet
        double stopTime @unit(s) = default(-1s);  // Time of finishing sending, negative values mean forever
        volatile double sendInterval @unit(s) = default(10ms); // May be a random value, e.g. exponential(1)
        int numPackets = default(-1); // Max number of packets to generate, -1 means forever
        int protocol; // Value for IPv4, IPv6 protocol field, accepts only between 143 and 254
        volatile int packetLength @unit(B); // Packet length in bytes
        string destAddresses = default(""); // List of destination addresses, separated by spaces
        @lifecycleSupport;
        @display("i=block/source");
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,vector; interpolationmode=none);
    gates:
        input ipIn @labels(Ipv4ControlInfo/up,Ipv6ControlInfo/up);
        output ipOut @labels(Ipv4ControlInfo/down,Ipv6ControlInfo/down);
}
