//
// Copyright (C) 2008 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.sctpapp;

import inet.applications.contract.IApp;

//
// Specialized SCTP peer application designed for testing NAT traversal scenarios
// and SCTP's multi-homing capabilities in networks with address translation.
// Combines client and server functionality with additional parameters for NAT-specific
// features such as rendezvous connection establishment and multi-homed operation.
//
// Supports various connection establishment methods including standard client-server
// and rendezvous modes where both peers initiate connection simultaneously. Provides
// comprehensive control over SCTP-specific features including stream management,
// partial reliability, and stream reset capabilities. The peer identification parameters
// (ownName, peerName) enable proper addressing in complex NAT scenarios where
// traditional addressing may be ambiguous.
//
// @see ~SctpNatServer, ~SctpPeer, ~SctpSocket
//
simple SctpNatPeer like IApp
{
    parameters:
        string localAddress = default(""); // Comma-separated list of addresses
        int localPort = default(0); // Port number to listen on
        string connectAddress;  // Server address (may be symbolic)
        int connectPort; // Port number to connect to
        double startTime @unit(s) = default(1s); // Time first session begins
        int numRequestsPerSession = default(1);  // Number of requests sent per session
        int numPacketsToReceive = default(0);
        int requestLength = default(1452); // Length of a request (bytes)
        double thinkTime @unit(s) = default(0s); // Time gap between requests
        double waitToClose @unit(s) = default(0s); // Time to wait between last message sent and abort
        //# double reconnectInterval @unit(s) = default(0);  // If the connection breaks, wait this much before trying to reconnect
        int outboundStreams = default(1);
        int inboundStreams = default(1);
        bool echo = default(false);  // Echoes received packages when true
        bool ordered = default(true);
        int queueSize = default(0);
        int prMethod = default(0);  //0=NONE, 1=PR_TTL, 2=PR_RTX, 3=PR_PRIO, 4=PR_STRRST
        bool streamReset = default(false);
        double streamRequestTime @unit(s) = default(0s);
        int streamResetType = default(5);  //NO_RESET=5, RESET_OUTGOING=6, RESET_INCOMING=7, RESET_BOTH=8, SSN_TSN=9
        //bool streamAnswer = default(false);
        double prValue = default(0); //for PR-SCTP
        //double stopTime @unit(s) = default(0s);
        double echoDelay @unit(s) = default(0s);
        double delayFirstRead @unit(s) = default(0s);
        int numPacketsToSendPerClient = default(0);  // Number of requests sent per session
        int numPacketsToReceivePerClient = default(1);
        int ownName = default(0);
        int peerName = default(0);
        bool rendezvous = default(false);
        bool multi= default(false);
        string streamPriorities = default("");
    gates:
        input socketIn;
        output socketOut;
}
