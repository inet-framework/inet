//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

//
// Multi-connection TCP traffic sink that accepts and consumes incoming data
// without generating responses. Designed for testing network performance,
// throughput measurement, and traffic generation scenarios where only the
// receiving side behavior is of interest.
//
// Creates a separate thread module (TcpSinkAppThread) for each incoming connection,
// enabling detailed per-connection statistics collection and independent handling
// of multiple simultaneous connections. Supports both IPv4 and IPv6 networks with
// configurable reception rate limiting through the autoRead, readSize, and readDelay
// parameters. When rate limiting is enabled (autoRead=false), TCP flow control
// mechanisms become active, allowing realistic simulation of network congestion
// and backpressure effects.
//
// @see ~TcpServerHostApp, ~TcpSinkAppThread, ~UdpSink
//
module TcpSinkApp extends TcpServerHostApp
{
    parameters:
        serverThreadModuleType = default("inet.applications.tcpapp.TcpSinkAppThread");
        bool autoRead = default(true); // Whether to use "autoread" or "explicit-read" mode for TCP connection
        volatile int readSize @unit(B) = default(-1B);    // Used only with autoRead==false
        volatile double readDelay @unit(s) = default(-1s);    // Used only with autoRead==false; delay for issuing a READ command after previous READ was satisfied; -1 means immediately, 0 means zero delay
        @class(TcpSinkApp);
        @display("i=block/sink");
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none; checkSignals=false);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none; checkSignals=false);
}

//
// Handles a single incoming TCP connection for ~TcpSinkApp.
//
simple TcpSinkAppThread
{
    parameters:
        @display("i=block/sink");
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none);
}
