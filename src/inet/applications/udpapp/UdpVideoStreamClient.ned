//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.udpapp;

import inet.applications.contract.IApp;

//
// Video streaming client.
//
// This module sends one "video streaming request" to `serverAddress:serverPort` at time `startTime`
// and receives a stream from the ~UdpVideoStreamServer server.
//
// @see ~UdpVideoStreamServer
//
simple UdpVideoStreamClient like IApp
{
    parameters:
        int localPort = default(-1);  // Local port (-1: use ephemeral port)
        string serverAddress;  // Server address
        int serverPort;  // Server port
        double startTime @unit(s) = default(1s);
        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after the lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for the lifecycle stop operation
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[throughput](title="throughput"; unit=bps; source="throughput(packetReceived)"; record=vector);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,vector; interpolationmode=none);
    gates:
        input socketIn @labels(UdpCommand/up);
        output socketOut @labels(UdpCommand/down);
}

