//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

//
// Implements a TCP echo service that listens for incoming connections and echoes back
// received data with configurable modifications. Extends ~TcpServerHostApp to handle
// multiple concurrent connections by creating a separate thread (TcpEchoAppThread) for
// each client. Provides flexible echo behavior through the echoFactor parameter, which
// controls the response size relative to the received data, and echoDelay, which allows
// introducing a constant delay before sending responses.
//
// Supports both exact echoing (when echoFactor=1) and scaled responses (when echoFactor≠1),
// making it useful for various network testing scenarios including bandwidth testing,
// latency measurement, and TCP flow control evaluation. The optional rate limiting
// capability (controlled by autoRead, readSize, and readDelay parameters) enables
// simulation of constrained server resources and allows TCP flow control mechanisms
// to be exercised.
//
// Collects comprehensive statistics on data transfer including packet counts, bytes
// transferred, and end-to-end delays, providing metrics for analyzing TCP connection
// performance under different conditions.
//
// @see ~TcpServerHostApp, ~TcpEchoAppThread, ~TcpSocket
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
