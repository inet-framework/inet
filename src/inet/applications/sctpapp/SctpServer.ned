//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.sctpapp;

import inet.applications.contract.IApp;

//
// Server app for SCTP-based request-reply protocols.
// Handles a single session (and SCTP connection) at a time.
//
simple SctpServer like IApp
{
    parameters:
        string localAddress = default(""); // comma separated list of addresses
        int localPort = default(0); // port number to listen on
        bool echo = default(false);  // echoes received packages when true
        volatile double echoDelay @unit(s) = default(0s);
        double delayFirstRead @unit(s) = default(0s);
        volatile double readingInterval @unit(s) = default(0s);
        int messagesToPush = default(0);
        int numPacketsToSendPerClient = default(0); // number of requests sent per session
        int numPacketsToReceivePerClient = default(1);
        int requestLength = default(1452); // length of a request (bytes)
        volatile double thinkTime @unit(s) = default(0s); // time gap between requests
        double waitToClose @unit(s) = default(0s); //time to wait between last message sent and abort
        bool finishEndsSimulation = default(false);
        bool ordered = default(true);
        int inboundStreams = default(17);
        int outboundStreams = default(1);
        int queueSize = default(0);
        int prMethod = default(0);  //0=NONE, 1=PR_TTL, 2=PR_RTX, 3=PR_PRIO, 4=PR_STRRST
        double prValue = default(0); //for PR-SCTP
        bool streamReset = default(false);
        double streamRequestTime @unit(s) = default(0s);
        int streamToReset = default(0);
        string streamPriorities = default("");
    gates:
        input socketIn;
        output socketOut;
}

