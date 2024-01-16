//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

import inet.applications.contract.IApp;


//
// Accepts any number of incoming TCP connections, and discards whatever
// arrives on them. Compatible with both ~Ipv4 and ~Ipv6.
//
// Each incoming connection is handled by a separate, dynamically created module,
// which allows collecting per-connection statistics in a convenient way.
//
// By default, reading from the socket is not rate limited. To allow rate
// limiting, set autoRead=false, and use the readSize and readDelay parameters
// to set a rate limit. This will allow TCP flow control to come into effect.
//
module TcpSinkApp extends TcpServerApp
{
    parameters:
        listener.serverConnectionModuleType = default("inet.applications.tcpapp.TcpSinkAppThread");
        string localAddress = default(""); // may be left empty ("")
        int localPort = default(1000); // port number to listen on
        bool autoRead = default(true); // whether to use "autoread" or "explicit-read" mode for TCP connection
        volatile int readSize @unit(B) = default(-1B);    // used only with autoRead==false
        volatile double readDelay @unit(s) = default(-1s);    // used only with autoRead==false; delay for issuing a READ command after previous READ was satisfied; -1 means immediately, 0 means zero delay
        *.localAddress = this.localAddress;
        *.localPort = this.localPort;
        *.autoRead = this.autoRead;
        *.readSize = this.readSize;
        *.readDelay = this.readDelay;
        @display("i=block/sink");
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none; checkSignals=false);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none; checkSignals=false);
}

//
// Handles a single incoming TCP connection for ~TcpSinkApp.
//
simple TcpSinkAppThread like IApp
{
    parameters:
        volatile int readSize @unit(B) = default(-1B);    // used only with autoRead==false
        volatile double readDelay @unit(s) = default(-1s);    // used only with autoRead==false; delay for issuing a READ command after previous READ was satisfied; -1 means immediately, 0 means zero delay
        @display("i=block/sink");
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none);
    gates:
        input socketIn;
        output socketOut;
}
