//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.sctpapp;

import inet.applications.contract.IApp;

//
// Client app for SCTP-based request-reply protocols.
// Handles a single session (and SCTP connection) at a time.
//
simple SctpClient like IApp
{
    parameters:
        string localAddress = default(""); // comma separated list of addresses
        int localPort = default(0); // port number to listen on
        string connectAddress;  // server address (may be symbolic)
        int connectPort; // port number to connect to
        double startTime @unit(s) = default(1s); // time first session begins
        volatile int numRequestsPerSession = default(1);  // number of requests sent per session
        int numPacketsToReceive = default(0);
        int requestLength = default(1452); // length of a request (bytes)
        double thinkTime @unit(s) = default(0s); // time gap between requests
        double waitToClose @unit(s) = default(0s); //time to wait between last message sent and abort
        double reconnectInterval @unit(s) = default(0s);  // if connection breaks, waits this much before trying to reconnect
        int inboundStreams = default(17);
        int outboundStreams = default(1);
        bool echo = default(false);  // echoes received packages when true
        bool ordered = default(true);
        int queueSize = default(0);
        int prMethod = default(0);  //0=NONE, 1=PR_TTL, 2=PR_RTX, 3=PR_PRIO, 4=PR_STRRST
        double prValue = default(0); //for PR-SCTP
        bool streamReset = default(false);
        double streamRequestTime @unit(s) = default(0s);
        int streamResetType = default(5);  //NO_RESET=5, RESET_OUTGOING=6, RESET_INCOMING=7, RESET_BOTH=8, SSN_TSN=9
        int streamToReset = default(0);
        double stopTime @unit(s) = default(-1s);  // time of finishing sending, negative values mean forever
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

