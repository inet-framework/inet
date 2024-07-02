//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

//
// Manages an established TCP connection. It handles data transmission to and
// from a module connected via the `trafficIn` and `trafficOut` gates. When data is
// received on the `trafficIn` gate, it is forwarded on the TCP connection via the
// `socketOut` gate, and vice versa. This module simplifies the task of data
// exchange over an established TCP connection.
//
// Note that reading from the socket is only rate limited if the connection was
// originally created with the autoRead=false setting. Then, you can use the
// `readSize` and `readDelay` parameters to set a rate limit. This will allow TCP
// flow control to come into effect.
//
// @see ~TcpServerConnection, ~TcpServerListener, ~TcpClientSocketIo
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
