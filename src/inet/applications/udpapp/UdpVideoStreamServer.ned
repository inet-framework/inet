//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.udpapp;

import inet.applications.contract.IApp;

//
// Provides on-demand video streaming services over UDP, responding to client requests
// with configurable video streams. Upon receiving a request, generates a stream of
// specified size and transmits it as a sequence of UDP packets to the requesting client.
// The transmission characteristics (packet size, sending interval, total stream size)
// can be configured with deterministic or stochastic parameters to model various
// streaming scenarios from constant bitrate (CBR) to variable bitrate (VBR) traffic.
//
// Supports concurrent streaming to multiple clients, maintaining separate state for
// each active stream. The server tracks statistics including the number of streams
// served, packets sent, and total bytes requested. The implementation focuses on
// traffic pattern generation rather than actual video encoding, making it suitable
// for network performance studies without the computational overhead of real video
// processing.
//
// @see ~UdpVideoStreamClient, ~UdpSocket
//
simple UdpVideoStreamServer like IApp
{
    parameters:
        int localPort; // Port to listen on
        volatile double sendInterval @unit(s); // Interval between sending video stream packets
        volatile int packetLen @unit(B);  // Length of a video packet in bytes
        volatile int videoSize @unit(B);  // Length of the full video stream in bytes
        int timeToLive = default(-1); // If not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        int dscp = default(-1); // If not -1, set the DSCP field (on IPv4/IPv6) of sent packets to this value
        int tos = default(-1); // If not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after the lifecycle stop operation is finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for the lifecycle stop operation
        @signal[packetSent](type=inet::Packet);
        @signal[reqStreamBytes](type=long);
        @statistic[reqStreamBytes](title="requested stream bytes"; record=count,sum,vector; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input socketIn @labels(UdpCommand/up);
        output socketOut @labels(UdpCommand/down);
}
