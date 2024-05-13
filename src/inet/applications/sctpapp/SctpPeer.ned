//
// Copyright (C) 2008 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.sctpapp;

import inet.applications.contract.IApp;

simple SctpPeer like IApp
{
    parameters:
        string localAddress = default(""); // Comma separated list of addresses
        int localPort = default(0); // Port number to listen on
        bool echo = default(false);  // Echoes received packages when true
        double echoDelay @unit(s) = default(0s);
        double delayFirstRead @unit(s) = default(0s);
        int numPacketsToSendPerClient = default(0); // Number of requests sent per session
        int numPacketsToReceivePerClient = default(1);
        int requestLength = default(1452); // Length of a request (bytes)
        double thinkTime @unit(s) = default(0s); // Time gap between requests
        double waitToClose @unit(s) = default(0s); //time to wait between last message sent and abort
        bool ordered = default(true);
        int outboundStreams = default(1);
        int inboundStreams = default(17);
        int queueSize = default(0);
        int prMethod = default(0);  //0=NONE, 1=PR_TTL, 2=PR_RTX, 3=PR_PRIO, 4=PR_STRRST
        double prValue = default(0); //for PR-SCTP
        bool streamReset = default(false);
        string connectAddress;  // Server address (may be symbolic)
        int connectPort; // Port number to connect to
        double startTime @unit(s) = default(1s); // Time first session begins
        int numRequestsPerSession = default(1);  // Number of requests sent per session
        int numPacketsToReceive = default(0);
        string streamPriorities = default("");

        @display("i=block/app");
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @signal[echoedPk](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[echoedPk](title="packets echoed"; source=echoedPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input socketIn @labels(SctpCommand/up);
        output socketOut @labels(SctpCommand/down);
}

