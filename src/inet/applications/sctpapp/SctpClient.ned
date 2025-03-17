//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.sctpapp;

import inet.applications.contract.IApp;

//
// Implements a configurable SCTP client application that manages connection
// establishment, data exchange, and connection termination with an SCTP server.
// Provides comprehensive control over SCTP-specific features including multi-homing,
// multi-streaming, partial reliability, and stream reset capabilities, enabling
// detailed simulation of advanced SCTP communication scenarios.
//
// Supports various operational modes including request-response patterns with
// configurable message sizes and timing, echo functionality, and stream prioritization.
// The client can be configured to establish connections at specific times, send a
// defined number of requests, and manage connection teardown with precise timing
// control. Maintains detailed statistics on packet transmission, reception, and
// echoing for performance analysis.
//
// @see ~SctpServer, ~SctpSocket
//
simple SctpClient like IApp
{
    parameters:
        string localAddress = default(""); // Comma-separated list of addresses
        int localPort = default(0); // Port number to listen on
        string connectAddress;  // Server address (may be symbolic)
        int connectPort; // Port number to connect to
        double startTime @unit(s) = default(1s); // Time the first session begins
        volatile int numRequestsPerSession = default(1);  // Number of requests sent per session
        int numPacketsToReceive = default(0);
        int requestLength = default(1452); // Length of a request (bytes)
        double thinkTime @unit(s) = default(0s); // Time gap between requests
        double waitToClose @unit(s) = default(0s); // Time to wait between the last message sent and abort
        double reconnectInterval @unit(s) = default(0s);  // If the connection breaks, waits this much before trying to reconnect
        int inboundStreams = default(17);
        int outboundStreams = default(1);
        bool echo = default(false);  // Echoes received packages when true
        bool ordered = default(true);
        int queueSize = default(0);
        int prMethod = default(0);  // 0=NONE, 1=PR_TTL, 2=PR_RTX, 3=PR_PRIO, 4=PR_STRRST
        double prValue = default(0); // For PR-SCTP
        bool streamReset = default(false);
        double streamRequestTime @unit(s) = default(0s);
        int streamResetType = default(5);  // NO_RESET=5, RESET_OUTGOING=6, RESET_INCOMING=7, RESET_BOTH=8, SSN_TSN=9
        int streamToReset = default(0);
        double stopTime @unit(s) = default(-1s);  // Time of finishing sending, negative values mean forever
        double primaryTime @unit(s) = default(0s);
        string newPrimary = default("");
        string streamRequestLengths = default("");
        string streamRequestRatio = default("");
        string streamPriorities = default("");
        bool finishEndsSimulation = default(false);
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @signal[echoedPk](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[echoedPk](title="packets echoed"; source=echoedPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);

    gates:
        input socketIn;
        output socketOut;
}
