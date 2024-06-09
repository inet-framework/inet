//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

import inet.applications.contract.IApp;

//
// Generic server application for modeling TCP-based request-reply style
// protocols or applications.
//
// The module listens on the designated TCP port, accepting incoming TCP
// connections. When a TCP connection is established, the model expects to
// receive messages of class ~GenericAppMsg on it. A message should contain how
// large the reply should be (number of bytes). For each received
// ~GenericAppMsg, ~TcpGenericServerApp will create another ~GenericAppMsg with
// the requested length as a response and send it back. The reply can be delayed
// by a constant time (`replyDelay` parameter).
//
// By default, the server does not impose rate limiting on data reads from the
// socket. To enable rate limiting, you can set `autoRead` to "false" and
// configure the `readSize` and `readDelay` parameters, thereby facilitating TCP
// flow control.
//
// Compatible with both ~Ipv4 and ~Ipv6.
//
// @see ~GenericAppMsg, ~TcpBasicClientApp, ~TelnetApp
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

