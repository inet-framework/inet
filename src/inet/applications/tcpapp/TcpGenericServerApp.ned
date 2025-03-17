//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

import inet.applications.contract.IApp;

//
// Provides a generic server implementation for TCP-based request-response protocols,
// designed to work with clients like ~TcpBasicClientApp. Listens for incoming connections
// on a configurable port and processes ~GenericAppMsg messages from clients. Each received
// message specifies the desired response size, which the server uses to generate and send
// back appropriately sized replies.
//
// Offers flexible configuration options including optional response delays and rate-limited
// socket reading. When rate limiting is enabled (by setting autoRead=false and configuring
// readSize/readDelay parameters), the server can simulate processing constraints and enable
// TCP flow control mechanisms to regulate traffic. This allows for modeling various server
// behaviors from high-performance servers to resource-constrained systems.
//
// Supports both IPv4 and IPv6 addressing, making it suitable for diverse network simulation
// scenarios. The simple request-response behavior can be used as a foundation for modeling
// various application protocols like HTTP, FTP, or custom client-server interactions.
//
// @see ~GenericAppMsg, ~TcpBasicClientApp, ~TelnetApp, ~TcpServerHostApp
//
simple TcpGenericServerApp like IApp
{
    parameters:
        string localAddress = default(""); // Local address; it may be left empty ("")
        int localPort = default(1000);     // LocalPort number to listen on
        bool autoRead = default(true); // Whether to use "autoread" or "explicit-read" mode for TCP connection
        volatile int readSize @unit(B) = default(-1B);    // Used only with autoRead==false
        volatile double readDelay @unit(s) = default(-1s);    // Used only with autoRead==false; delay for issuing a READ command after the previous READ was satisfied; -1 means immediately, 0 means zero delay
        double replyDelay @unit(s) = default(0s);
        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for the lifecycle stop operation
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none);
    gates:
        input socketIn @labels(TcpCommand/up);
        output socketOut @labels(TcpCommand/down);
}
