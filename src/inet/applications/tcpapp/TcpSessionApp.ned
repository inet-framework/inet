//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

import inet.applications.contract.IApp;

//
// Flexible TCP application that manages a single connection with precise timing
// control for connection establishment, data transmission, and connection closure.
// Can function in both client and server modes, supporting IPv4 and IPv6 networks
// with configurable traffic patterns and data transfer modes.
//
// Provides comprehensive control over TCP session lifecycle through parameters
// that determine connection timing, data volume, and transmission scheduling.
// In client mode (active=true), initiates connections to specified remote endpoints,
// while in server mode (active=false) accepts a single incoming connection on the
// configured local port. Data transmission can be configured either as a one-time
// transfer of a specified volume at a set time, or through a script defining a
// sequence of timed transmissions.
//
// Supports three data transfer modes: "bytecount" for simulating traffic volume
// without actual data content, "object" for preserving message boundaries and
// object identities across the connection, and "bytestream" for transmitting
// actual byte content. Includes configurable reception rate limiting capabilities
// through the autoRead, readSize, and readDelay parameters, enabling realistic
// flow control behavior. Collects comprehensive statistics on traffic patterns
// including packet counts, bytes transferred, and end-to-end delay measurements.
//
// @see ~TcpSocket, ~TcpClientApp, ~TcpServerApp, ~TcpGenericServerApp
//
simple TcpSessionApp like IApp
{
    parameters:
        string localAddress = default("");
        int localPort = default(-1);  // Local port
        bool active = default(true);
        string connectAddress;
        int connectPort = default(1000);
        string dataTransferMode @enum("bytecount","object","bytestream") = default("bytecount");
        bool autoRead = default(true); // Whether to use "autoread" or "explicit-read" mode for TCP connection
        volatile int readSize @unit(B) = default(-1B);    // Used only with autoRead==false
        volatile double readDelay @unit(s) = default(-1s);    // Used only with autoRead==false; delay for issuing a READ command after previous READ was satisfied; -1 means immediately, 0 means zero delay
        double tOpen @unit(s) = default(1s);
        double tSend @unit(s) = default(1s);
        int sendBytes @unit(B) = default(1MiB);
        string sendScript = default("");
        double tClose @unit(s) = default(2s);
        int timeToLive = default(-1); // If not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        int dscp = default(-1); // If not -1, set the DSCP (IPv4/IPv6) field of sent packets to this value
        int tos = default(-1); // If not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
        @signal[connect](type=long);
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none);
    gates:
        input socketIn @labels(TcpCommand/up);
        output socketOut @labels(TcpCommand/down);
}
