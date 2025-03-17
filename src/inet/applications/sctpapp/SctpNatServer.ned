//
// Copyright (C) 2008 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.sctpapp;

import inet.applications.contract.IApp;

//
// Specialized SCTP server application designed for testing NAT traversal scenarios
// in networks with address translation. Provides a simplified server implementation
// that listens for incoming connections from SctpNatPeer instances and processes
// their requests in NAT environments.
//
// Features a streamlined parameter set focused on essential server functionality
// including multi-streaming support and message exchange configuration. The server
// can be configured to process a specific number of packets per client and send
// responses with precise timing control. Particularly useful for testing SCTP's
// ability to establish and maintain connections through NAT devices and evaluating
// protocol performance in address-translated network environments.
//
// @see ~SctpNatPeer, ~SctpServer, ~SctpSocket
//
simple SctpNatServer like IApp
{
    parameters:
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
