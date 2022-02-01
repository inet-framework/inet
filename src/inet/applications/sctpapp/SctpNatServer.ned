//
// Copyright (C) 2008 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.sctpapp;

import inet.applications.contract.IApp;

simple SctpNatServer like IApp
{
    parameters:
        string localAddress = default(""); // may be left empty ("")
        int localPort = default(0); // port number to listen on
        int numPacketsToSendPerClient = default(0);  // number of requests sent per session
        int numPacketsToReceivePerClient = default(1);
        int requestLength = default(1452); // length of a request (bytes)
        double waitToClose @unit(s) = default(0s); //time to wait between last message sent and abort
        int outboundStreams = default(1);
        int inboundStreams = default(1);
        bool ordered = default(true);
    gates:
        input socketIn;
        output socketOut;
}

