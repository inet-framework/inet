//
// Copyright (C) 2008 Irene Ruengeler
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


simple SCTPPeer like ISCTPApp
{
    parameters:
        string localAddress = default(""); // comma separated list of addresses
        int localPort = default(0); // port number to listen on
        bool echo = default(false);  // echoes received packages when true
        double echoDelay @unit(s) = default(0s);
        double delayFirstRead @unit(s) = default(0s);
        int numPacketsToSendPerClient = default(0); // number of requests sent per session
        int numPacketsToReceivePerClient = default(1);
        int requestLength = default(1452); // length of a request (bytes)
        double thinkTime @unit(s) = default(0s); // time gap between requests
        double waitToClose @unit(s) = default(0s); //time to wait between last message sent and abort
        bool ordered = default(true);
        int outboundStreams = default(1);
        int inboundStreams = default(17);
        int queueSize = default(0);
        int prMethod = default(0);  //0=NONE, 1=PR_TTL, 2=PR_RTX, 3=PR_PRIO, 4=PR_STRRST
        double prValue = default(0); //for PR-SCTP
        bool streamReset = default(false);
        string connectAddress;  // server address (may be symbolic)
        int connectPort; // port number to connect to
        double startTime @unit(s) = default(1s); // time first session begins
        int numRequestsPerSession = default(1);  // number of requests sent per session
        int numPacketsToReceive = default(0);
        string streamPriorities = default("");

        @display("i=block/app");
        @signal[sentPk](type=SCTPSimpleMessage);
        @signal[rcvdPk](type=cPacket);
        @signal[echoedPk](type=SCTPSimpleMessage);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[echoedPk](title="packets echoed"; source=echoedPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input sctpIn @labels(SCTPCommand/up);
        output sctpOut @labels(SCTPCommand/down);
}

