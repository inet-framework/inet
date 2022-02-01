//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

import inet.applications.contract.IApp;

//
// Models Telnet sessions with a specific user behaviour.
// The server app should be ~TcpGenericServerApp.
// Compatible with both ~Ipv4 and ~Ipv6.
//
// NOTE: This module emulates a very specific user behaviour, and as such,
// it should be viewed as an example rather than a generic Telnet model.
// If you want to model realistic Telnet traffic, you are encouraged
// to gather statistics from packet traces on a real network, and
// write your model accordingly.
//
simple TelnetApp like IApp
{
    parameters:
        string localAddress = default(""); // local address or empty ("")
        int localPort; // local port number
        string connectAddress;  // server address (may be symbolic)
        int connectPort = default(1000); // port number to connect to
        double startTime @unit(s) = default(1s); // time first session begins
        double stopTime @unit(s) = default(-1s);  // time of finishing sending, negative values mean forever
        volatile int numCommands = default(int(exponential(10))); // user types this many commands in a session
        volatile int commandLength @unit(B) = default(10B); // commands are this many characters (plus Enter)
        volatile double keyPressDelay @unit(s) = default(exponential(0.1s)); // delay between keypresses
        volatile int commandOutputLength @unit(B) = default(intWithUnit(exponential(1000B))); // commands produce this much output
        volatile double thinkTime @unit(s) = default(exponential(10s)); // user waits this much before starting to type new command
        volatile double idleInterval @unit(s) = default(exponential(300s)); // time gap between sessions
        volatile double reconnectInterval @unit(s) = default(30s);  // if connection breaks, user waits this much before trying to reconnect
        int timeToLive = default(-1); // if not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        int dscp = default(-1); // if not -1, set the ToS (IPv4) or Traffic Class (IPv6) field of sent packets to this value
        int tos = default(-1); // if not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        @display("i=block/telnet");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @signal[connect](type=long);  // 1 for open, -1 for close
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none);
        @statistic[numActiveSessions](title="number of active sessions"; source=warmup(sum(connect)); record=max,timeavg,vector; interpolationmode=sample-hold; autoWarmupFilter=false);
        @statistic[numSessions](title="total number of sessions"; source="sum(connect+1)/2"; record=last);

    gates:
        input socketIn @labels(TcpCommand/up);
        output socketOut @labels(TcpCommand/down);
}

