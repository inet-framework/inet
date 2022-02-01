//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.udpapp;

import inet.applications.contract.IApp;

//
// Video stream server. To be used with ~UdpVideoStreamClient.
//
// The server will wait for incoming "video streaming requests".
// When a request arrives, it draws a random video stream size
// using the videoSize parameter, and starts streaming to the client.
// During streaming, it will send UDP packets of size packetLen at every
// sendInterval, until videoSize is reached. The parameters packetLen
// and sendInterval can be set to constant values to create CBR traffic,
// or to random values (e.g. sendInterval=uniform(1e-6, 1.01e-6)) to
// accomodate jitter.
//
// The server can serve several clients, and several streams per client.
//
// @see ~UdpVideoStreamClient
//
simple UdpVideoStreamServer like IApp
{
    parameters:
        int localPort; // port to listen on
        volatile double sendInterval @unit(s); // interval between sending video stream packets
        volatile int packetLen @unit(B);  // length of a video packet in bytes
        volatile int videoSize @unit(B);  // length of full a video stream in bytes
        int timeToLive = default(-1); // if not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        int dscp = default(-1); // if not -1, set the DSCP field (on IPv4/IPv6) of sent packets to this value
        int tos = default(-1); // if not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        @signal[packetSent](type=inet::Packet);
        @signal[reqStreamBytes](type=long);
        @statistic[reqStreamBytes](title="requested stream bytes"; record=count,sum,vector; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}

