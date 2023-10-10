//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

import inet.applications.contract.IApp;


//
// Accepts any number of incoming TCP connections, and sends back the data that
// arrives on them. The echoFactor parameter controls the amount of data to be
// sent back. If echoFactor==1, the received data is echoed back without change.
// For any other positive value of echoFactor, a message of appropriate length
// (datalen*echoFactor) without content (see ByteCountChunk C++ class) is sent
// back.
//
// The reply can also be delayed by a constant time (echoDelay parameter).
//
// By default, reading from the socket is not rate limited. To allow rate
// limiting, set autoRead=false, and use the readSize and readDelay parameters
// to set a rate limit. This will allow TCP flow control to come into effect.
//
// Compatible with both ~Ipv4 and ~Ipv6.
//
module TcpEchoApp extends TcpServerApp
{
    parameters:
        listener.serverConnectionModuleType = default("inet.applications.tcpapp.TcpEchoAppThread");
        string localAddress = default(""); // may be left empty ("")
        int localPort = default(1000); // port number to listen on
        bool autoRead = default(true); // whether to use "autoread" or "explicit-read" mode for TCP connection
        volatile int readSize @unit(B) = default(-1B);    // used only with autoRead==false
        volatile double readDelay @unit(s) = default(-1s);    // used only with autoRead==false; delay for issuing a READ command after previous READ was satisfied; -1 means immediately, 0 means zero delay
        double echoFactor = default(1);
        double echoDelay @unit(s) = default(0s);
        *.localAddress = this.localAddress;
        *.localPort = this.localPort;
        *.autoRead = this.autoRead;
        *.readSize = this.readSize;
        *.readDelay = this.readDelay;
        *.echoFactor = this.echoFactor;
        *.echoDelay = this.echoDelay;
        //@display("i=block/sink");
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none; checkSignals=false);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none; checkSignals=false);
}

// Thread for ~TcpEchoApp
simple TcpEchoAppThread like IApp
{
    parameters:
        bool autoRead = default(true); // whether to use "autoread" or "explicit-read" mode for TCP connection
        volatile int readSize @unit(B) = default(-1B);    // used only with autoRead==false
        volatile double readDelay @unit(s) = default(-1s);    // used only with autoRead==false; delay for issuing a READ command after previous READ was satisfied; -1 means immediately, 0 means zero delay
        double echoFactor = default(1);
        double echoDelay @unit(s) = default(0s);
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none);
    gates:
        input socketIn;
        output socketOut;
}
