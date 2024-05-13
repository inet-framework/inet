//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

import inet.applications.contract.IApp;

//
// Models Telnet sessions with a specific user behavior.
// The server app should be ~TcpGenericServerApp.
// Compatible with both ~Ipv4 and ~Ipv6.
//
// NOTE: This module emulates a very specific user behavior, and as such,
// it should be viewed as an example rather than a generic Telnet model.
// If you want to model realistic Telnet traffic, you are encouraged
// to gather statistics from packet traces on a real network, and
// write your model accordingly.
//
// By default, reading from the socket is not rate limited. To allow rate
// limiting, set autoRead=false, and use the readSize and readDelay parameters
// to set a rate limit. This will allow TCP flow control to come into effect.
//
simple TelnetApp like IApp
{
    parameters:
        string localAddress = default(""); // Local address or empty ("")
        int localPort; // Local port number
        string connectAddress;  // Server address (may be symbolic)
        int connectPort = default(1000); // Port number to connect to
        bool autoRead = default(true); // Whether to use "autoread" or "explicit-read" mode for TCP connection
        volatile int readSize @unit(B) = default(-1B);    // Used only with autoRead==false
        volatile double readDelay @unit(s) = default(-1s);    // Used only with autoRead==false; delay for issuing a READ command after previous READ was satisfied; -1 means immediately, 0 means zero delay
        double startTime @unit(s) = default(1s); // Time first session begins
        double stopTime @unit(s) = default(-1s);  // Time of finishing sending, negative values mean forever
        volatile int numCommands = default(int(exponential(10))); // User types this many commands in a session
        volatile int commandLength @unit(B) = default(10B); // Commands are this many characters (plus Enter)
        volatile double keyPressDelay @unit(s) = default(exponential(0.1s)); // Delay between keypresses
        volatile int commandOutputLength @unit(B) = default(intWithUnit(exponential(1000B))); // Commands produce this much output
        volatile double thinkTime @unit(s) = default(exponential(10s)); // User waits this much before starting to type new command
        volatile double idleInterval @unit(s) = default(exponential(300s)); // Time gap between sessions
        volatile double reconnectInterval @unit(s) = default(30s);  // If connection breaks, user waits this much before trying to reconnect
        int timeToLive = default(-1); // If not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        int dscp = default(-1); // If not -1, set the ToS (IPv4) or Traffic Class (IPv6) field of sent packets to this value
        int tos = default(-1); // If not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        @display("i=block/telnet");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
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

