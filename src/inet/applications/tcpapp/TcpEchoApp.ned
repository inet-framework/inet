//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

//
// Accepts any number of incoming TCP connections, and sends back the data that
// arrives on them. The `echoFactor` parameter controls the amount of data to be
// sent back. If echoFactor==1, the received data is echoed back without change.
// For any other positive value of `echoFactor`, a message of appropriate length
// (datalen*`echoFactor`) without content (see `ByteCountChunk` C++ class) is sent
// back.
//
// The reply can also be delayed by a constant time (`echoDelay` parameter).
//
// By default, reading from the socket is not rate-limited. To allow rate
// limiting, set autoRead=false, and use the `readSize` and `readDelay` parameters
// to set a rate limit. This will allow TCP flow control to come into effect.
//
// Compatible with both ~Ipv4 and ~Ipv6.
//
simple TcpEchoApp extends TcpServerHostApp
{
    parameters:
        @class(TcpEchoApp);
        bool autoRead = default(true); // Whether to use "autoread" or "explicit-read" mode for TCP connection
        volatile int readSize @unit(B) = default(-1B);    // Used only with autoRead==false
        volatile double readDelay @unit(s) = default(-1s);    // Used only with autoRead==false; delay for issuing a READ command after the previous READ was satisfied; -1 means immediately, 0 means zero delay
        double echoFactor = default(1);
        double echoDelay @unit(s) = default(0s);
        serverThreadModuleType = default("inet.applications.tcpapp.TcpEchoAppThread"); // Module type of the "thread" module to launch on incoming connections
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none);
}

// Thread for ~TcpEchoApp
simple TcpEchoAppThread
{
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none);
}
