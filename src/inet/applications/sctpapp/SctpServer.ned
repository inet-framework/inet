//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.sctpapp;

import inet.applications.contract.IApp;

//
// Implements a configurable SCTP server application that listens for incoming
// connections and processes client requests. Provides extensive control over
// SCTP-specific features including multi-streaming, partial reliability, and
// stream reset capabilities, enabling detailed simulation of advanced SCTP
// communication scenarios.
//
// Supports various operational modes including request-response patterns with
// configurable message sizes and timing, echo functionality with adjustable delay,
// and controlled reading behavior. The server can be configured to process a specific
// number of packets per client, send responses with precise timing control, and
// manage connection teardown. Particularly useful for testing SCTP protocol features
// and evaluating application performance under different network conditions.
//
// @see ~SctpClient, ~SctpSocket
//
simple SctpServer like IApp
{
    parameters:
        string localAddress = default(""); // Comma-separated list of addresses
        int localPort = default(0); // Port number to listen on
        bool echo = default(false);  // Echoes received packages when true
        volatile double echoDelay @unit(s) = default(0s);
        double delayFirstRead @unit(s) = default(0s);
        volatile double readingInterval @unit(s) = default(0s);
        int messagesToPush = default(0);
        int numPacketsToSendPerClient = default(0); // Number of requests sent per session
        int numPacketsToReceivePerClient = default(1);
        int requestLength = default(1452); // Length of a request (bytes)
        volatile double thinkTime @unit(s) = default(0s); // Time gap between requests
        double waitToClose @unit(s) = default(0s); // Time to wait between last message sent and abort
        bool finishEndsSimulation = default(false);
        bool ordered = default(true);
        int inboundStreams = default(17);
        int outboundStreams = default(1);
        int queueSize = default(0);
        int prMethod = default(0);  // 0=NONE, 1=PR_TTL, 2=PR_RTX, 3=PR_PRIO, 4=PR_STRRST
        double prValue = default(0); // For PR-SCTP
        bool streamReset = default(false);
        double streamRequestTime @unit(s) = default(0s);
        int streamToReset = default(0);
        string streamPriorities = default("");
    gates:
        input socketIn;
        output socketOut;
}
