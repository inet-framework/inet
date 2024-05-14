//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

//
// This module opens a TCP connection to a given address and port and lets the
// module connected to its trafficIn and trafficOut gates send and receive
// data over that connection. When data is received on the trafficIn gate, it is
// forwarded on the TCP connection via the socketOut gate, and vice versa. This
// module simplifies the task of data exchange over a TCP connection. The TCP
// connection is opened when the module first receives data on its trafficIn
// gate.
//
// By default, reading from the socket is not rate limited. To allow rate
// limiting, set autoRead=false, and use the readSize and readDelay parameters
// to set a rate limit. This will allow TCP flow control to come into effect.
//
// @see ~TcpServerSocketIo
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
