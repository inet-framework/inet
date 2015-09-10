//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.applications.sctpapp;

import inet.applications.contract.ISCTPApp;

//
// Client app for SCTP-based request-reply protocols.
// Handles a single session (and SCTP connection) at a time.
//
simple SCTPClient like ISCTPApp
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
        double reconnectInterval @unit(s) = default(0);  // if connection breaks, waits this much before trying to reconnect
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
        bool streamAnswer = default(false);
        double stopTime @unit(s) = default(-1s);  // time of finishing sending, negative values mean forever
        double primaryTime @unit(s) = default(0s);
        string newPrimary = default("");
        string streamRequestLengths = default("");
        string streamRequestRatio = default("");
        string streamPriorities = default("");
        bool finishEndsSimulation = default(false);
        @signal[sentPk](type=SCTPSimpleMessage);
        @signal[rcvdPk](type=cPacket);
        @signal[echoedPk](type=SCTPSimpleMessage);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[echoedPk](title="packets echoed"; source=echoedPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);

    gates:
        input sctpIn;
        output sctpOut;
}

