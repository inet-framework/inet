package inet.applications.sctpapp;
// Copyright (C) 2008 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//
// Accepts any number of incoming TCP connections, and sends back the
// messages that arrive on them, The lengths of the messages are
// multiplied by echoFactor before sending them back (echoFactor=1 will
// result in sending back the same message unmodified.) The reply can also be
// delayed by a constant time (echoDelay parameter).
//
// When TCPEchoApp receives data packets from TCP (and such, when they can be
// echoed) depends on the sendQueue/receiveQueue setting of TCPMain.
// With TCPVirtualBytesSendQueue/RcvQueue, TCP passes up data to us
// as soon as a segment arrives, so it can be echoed immediately.
// With TCPMsgBasedSendQueue/RcvQueue, our local TCP reproduces the same
// messages that the sender app passed down to its TCP -- so if the sender
// app sent a single 100 MB message, it will be echoed only when all
// 100 megabytes have arrived.
//
import inet.applications.contract.ISCTPApp;

simple SCTPNatPeer like ISCTPApp
{
    parameters:
        string localAddress = default(""); // comma separated list of addresses
        int localPort = default(0); // port number to listen on
        string connectAddress;  // server address (may be symbolic)
        int connectPort; // port number to connect to
        double startTime @unit(s) = default(1s); // time first session begins
        int numRequestsPerSession = default(1);  // number of requests sent per session
        int numPacketsToReceive = default(0);
        int requestLength = default(1452); // length of a request (bytes)
        double thinkTime @unit(s) = default(0s); // time gap between requests
        double waitToClose @unit(s) = default(0s); //time to wait between last message sent and abort
        //double reconnectInterval @unit(s) = default(0);  // if connection breaks, waits this much before trying to reconnect
        int outboundStreams = default(1);
        int inboundStreams = default(1);
        bool echo = default(false);  // echoes received packages when true
        bool ordered = default(true);
        int queueSize = default(0);
        int prMethod = default(0);  //0=NONE, 1=PR_TTL, 2=PR_RTX, 3=PR_PRIO, 4=PR_STRRST
        bool streamReset = default(false);
        double streamRequestTime @unit(s) = default(0s);
        int streamResetType = default(5);  //NO_RESET=5, RESET_OUTGOING=6, RESET_INCOMING=7, RESET_BOTH=8, SSN_TSN=9
        bool streamAnswer = default(false);
        double prValue = default(0); //for PR-SCTP
        //double stopTime @unit(s) = default(0s);
        double echoDelay @unit(s) = default(0s);
        double delayFirstRead @unit(s) = default(0s);
        int numPacketsToSendPerClient = default(0);  // number of requests sent per session
        int numPacketsToReceivePerClient = default(1);
        int ownName = default(0);
        int peerName = default(0);
        bool rendezvous = default(false);
        bool multi= default(false);
        string streamPriorities = default("");
    gates:
        input sctpIn;
        output sctpOut;
}