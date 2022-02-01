//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

import inet.applications.contract.IApp;

//
// Generic server application for modelling TCP-based request-reply style
// protocols or applications.
//
// The module accepts any number of incoming TCP connections, and expects
// to receive messages of class ~GenericAppMsg on them. A message should
// contain how large the reply should be (number of bytes). ~TcpGenericServerApp
// will just change the length of the received message accordingly, and send
// back the same message object. The reply can be delayed by a constant time
// (replyDelay parameter).
//
// Compatible with both ~Ipv4 and ~Ipv6.
//
// @see ~GenericAppMsg, ~TcpBasicClientApp, ~TelnetApp
//
simple TcpGenericServerApp like IApp
{
    parameters:
        string localAddress = default(""); // local address; may be left empty ("")
        int localPort = default(1000);     // localPort number to listen on
        double replyDelay @unit(s) = default(0s);
        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none);
    gates:
        input socketIn @labels(TcpCommand/up);
        output socketOut @labels(TcpCommand/down);
}

