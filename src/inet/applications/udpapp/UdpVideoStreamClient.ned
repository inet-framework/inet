//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.udpapp;

import inet.applications.contract.IApp;

//
// Implements a client application for requesting and receiving video streams over UDP.
// Initiates the streaming process by sending a single request message to a configured
// server address and port at a specified start time. Upon receiving this request, the
// ~UdpVideoStreamServer begins transmitting the video stream as a sequence of UDP packets.
//
// Collects comprehensive statistics on the received stream, including packet count,
// throughput, and end-to-end delay measurements. These metrics are essential for
// evaluating network performance in video streaming scenarios. The client is designed
// to work with the ~UdpVideoStreamServer module, forming a complete video streaming
// simulation framework suitable for studying various network conditions and their
// impact on streaming applications.
//
// @see ~UdpVideoStreamServer, ~UdpSocket
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
