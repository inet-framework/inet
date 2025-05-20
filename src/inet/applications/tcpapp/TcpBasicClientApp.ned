//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

import inet.common.SimpleModule;
import inet.applications.contract.IApp;

//
// Client for a generic request-response style protocol over TCP, to be used
// with ~TcpGenericServerApp.
//
// The model communicates with the server in sessions. During a session, the
// client opens a single TCP connection to the server, sends several requests
// (always waiting for the complete reply to arrive before sending a new
// request), and closes the connection. Requests and responses are represented
// by messages of class ~GenericAppMsg.
//
// By default, reading from the socket is not rate limited. To allow rate
// limiting, set autoRead=false, and use the `readSize` and `readDelay` parameters
// to set a rate limit. This will allow TCP flow control to come into effect.
//
// Compatible with both IPv4 (~Ipv4) and IPv6 (~Ipv6).
//
// As an example, the following parameterization can be used as a very rough
// model of an HTTP session where the client downloads a file, or several files.
//
// <pre>
//    numRequestsPerSession = 1 <i>(HTTP 1.0)</i>
//    numRequestsPerSession = exponential(5) <i>(HTTP 1.1, with keepalive)</i>
//    requestLength = 1B*int(truncnormal(350,20))
//    replyLength = 1B*int(exponential(2000))
// </pre>
//
// @see ~TcpGenericServerApp, ~GenericAppMsg, ~TelnetApp
//
simple TcpBasicClientApp extends SimpleModule like IApp
{
    parameters:
        @class(TcpBasicClientApp);
        string localAddress = default(""); // May be left empty ("")
        int localPort = default(-1); // Port number to listen on
        string connectAddress = default("");  // Server address (may be symbolic)
        int connectPort = default(1000); // Port number to connect to
        bool autoRead = default(true); // Whether to use "autoread" or "explicit-read" mode for TCP connection
        volatile int readSize @unit(B) = default(-1B);    // Used only with autoRead==false
        volatile double readDelay @unit(s) = default(-1s);    // Used only with autoRead==false; delay for issuing a READ command after the previous READ was satisfied; -1 means immediately, 0 means zero delay
        double startTime @unit(s) = default(1s); // Time the first session begins
        double stopTime @unit(s) = default(-1s);  // Time of finishing sending, negative values mean forever
        volatile int numRequestsPerSession = default(1);  // Number of requests sent per session
        volatile int requestLength @unit(B) = default(200B); // Length of a request
        volatile int replyLength @unit(B) = default(1MiB); // Length of a reply
        volatile double thinkTime @unit(s); // Time gap between requests
        volatile double idleInterval @unit(s); // Time gap between sessions
        volatile double reconnectInterval @unit(s) = default(30s);  // If the connection breaks, wait this long before trying to reconnect
        int timeToLive = default(-1); // If not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        int dscp = default(-1); // If not -1, set the DSCP (IPv4/IPv6) field of sent packets to this value
        int tos = default(-1); // If not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after the lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for the lifecycle stop operation
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @signal[connect](type=long);  // 1 for open, -1 for close
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none);
        @statistic[numActiveSessions](title="number of active sessions"; source=warmup(sum(connect)); record=max,timeavg,vector; interpolationmode=sample-hold; autoWarmupFilter=false);
        @statistic[numSessions](title="total number of sessions"; source="sum(connect+1)/2"; record=last);
        @statistic[throughput](title="throughput of received data"; unit=bps; source="throughput(packetReceived)"; record=vector);
    gates:
        input socketIn @labels(TcpCommand/up);
        output socketOut @labels(TcpCommand/down);
}

