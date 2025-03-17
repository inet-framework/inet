//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

//
// Bidirectional adapter for established TCP server connections that bridges between
// application traffic and TCP socket communication. Provides a clean separation
// between application logic and network transport, enabling modular design of
// TCP-based server applications.
//
// Functions as a data relay that forwards incoming application data from the trafficIn
// gate to the TCP socket via socketOut, and delivers received TCP data from socketIn
// to the application via trafficOut. Supports configurable rate limiting through
// readSize and readDelay parameters when used with connections created with autoRead=false,
// enabling effective TCP flow control for managing network congestion and preventing
// buffer overflows. Typically instantiated by TcpServerConnection modules to handle
// the data exchange aspects of accepted connections.
//
// @see ~TcpServerConnection, ~TcpServerListener, ~TcpClientSocketIo, ~TcpSocket
//
simple TcpServerSocketIo
{
    parameters:
        @display("i=block/socket");
        volatile int readSize @unit(B) = default(-1B);    // Used only with autoRead==false
        volatile double readDelay @unit(s) = default(-1s);    // Used only with autoRead==false; delay for issuing a READ command after previous READ was satisfied; -1 means immediately, 0 means zero delay
    gates:
        input trafficIn;
        output trafficOut;
        input socketIn;
        output socketOut;
}
