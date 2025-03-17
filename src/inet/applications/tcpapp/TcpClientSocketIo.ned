//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

//
// Provides a transparent TCP client socket interface that bridges between application
// traffic and the TCP protocol. Establishes and manages a TCP connection to a specified
// server address and port, handling the details of connection setup and data transfer.
// The module acts as an adapter that converts between application-level traffic and
// TCP socket commands, simplifying the development of TCP-based applications.
//
// Operates in a lazy connection establishment mode, where the TCP connection is only
// initiated when the first data packet arrives on the trafficIn gate. This design
// allows applications to focus on data exchange logic without handling connection
// management details. All incoming data from the application is forwarded to the
// TCP connection, and all data received from the TCP connection is forwarded to
// the application.
//
// Supports optional rate limiting for reading from the socket through the autoRead,
// readSize, and readDelay parameters. When rate limiting is enabled, TCP flow control
// mechanisms can be exercised, allowing simulation of bandwidth-constrained clients
// or testing server behavior under various client reception conditions.
//
// @see ~TcpServerSocketIo, ~TcpClientApp, ~TcpSocket
//
simple TcpClientSocketIo
{
    parameters:
        string localAddress = default("");
        int localPort = default(-1);
        string connectAddress;
        int connectPort;
        bool autoRead = default(true); // Whether to use "autoread" or "explicit-read" mode for TCP connection
        volatile int readSize @unit(B) = default(-1B);    // Used only with autoRead==false
        volatile double readDelay @unit(s) = default(-1s);    // Used only with autoRead==false; delay for issuing a READ command after the previous READ was satisfied; -1 means immediately, 0 means zero delay
        @display("i=block/socket");
    gates:
        input trafficIn;
        output trafficOut;
        input socketIn;
        output socketOut;
}
