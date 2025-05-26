//
// Copyright (C) 2008 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.sctpapp;

import inet.common.SimpleModule;
import inet.applications.contract.IApp;

//
// Implements a rendezvous server for SCTP NAT traversal. Facilitates
// communication between peers behind NAT devices by maintaining connection
// information and exchanging address details between peers. Handles multi-homed
// connections and processes address addition notifications. Enables direct
// peer-to-peer communication by providing each peer with the necessary
// connection details of the other peer.
//
simple SctpNatServer extends SimpleModule like IApp
{
    parameters:
        @class(SctpNatServer);
        string localAddress = default(""); // May be left empty ("")
        int localPort = default(0); // Port number to listen on
        int numPacketsToSendPerClient = default(0);  // Number of requests sent per session
        int numPacketsToReceivePerClient = default(1);
        int requestLength = default(1452); // Length of a request (bytes)
        double waitToClose @unit(s) = default(0s); // Time to wait between last message sent and abort
        int outboundStreams = default(1);
        int inboundStreams = default(1);
        bool ordered = default(true);
    gates:
        input socketIn;
        output socketOut;
}

