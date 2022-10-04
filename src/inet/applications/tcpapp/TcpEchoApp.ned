//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

//
// Accepts any number of incoming TCP connections, and sends back the
// messages that arrive on them, The lengths of the messages are
// multiplied by echoFactor before sending them back (echoFactor=1 will
// result in sending back the same message unmodified.) The reply can also be
// delayed by a constant time (echoDelay parameter).
//
// Compatible with both ~Ipv4 and ~Ipv6.
//
simple TcpEchoApp extends TcpServerHostApp
{
    parameters:
        @class(TcpEchoApp);
        double echoFactor = default(1);
        double echoDelay @unit(s) = default(0s);
        serverThreadModuleType = default("inet.applications.tcpapp.TcpEchoAppThread"); // module type of "thread" module to launch on incoming connections
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none);
}

// Thread for ~TcpEchoApp
simple TcpEchoAppThread
{
}

