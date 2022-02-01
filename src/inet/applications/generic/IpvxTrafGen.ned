//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.generic;

//
// Sends IP or IPv6 datagrams to the given address at the given sendInterval.
// The sendInterval can be a constant or a random value (e.g. exponential(1)).
// If the destAddresses parameter contains more than one address, one
// of them is randomly for each packet. An address may be given in the
// dotted decimal notation (or, for IPv6, in the usual notation with colons),
// or with the module name. (The L3AddressResolver class is used to resolve
// the address.) To disable the model, set destAddresses to "".
//
// The peer can be ~IpvxTrafSink or another ~IpvxTrafGen (it handles received packets
// exactly like ~IpvxTrafSink).
//
simple IpvxTrafGen like IIpvxTrafficGenerator
{
    parameters:
        double startTime @unit(s) = default(1s); // time of sending the first packet
        double stopTime @unit(s) = default(-1s);  // time of finishing sending, negative values mean forever
        volatile double sendInterval @unit(s) = default(10ms); // may be a random value, e.g. exponential(1)
        int numPackets = default(-1); // max number of packets to generate, -1 means forever
        int protocol; // value for IPv4, IPv6 protocol field, accepts only between 143 and 254
        volatile int packetLength @unit(B); // packet length in bytes
        string destAddresses = default(""); // list of destination addresses, separated by spaces
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

