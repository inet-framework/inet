//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

//
// This module provides TCP client socket handling for generic TCP applications.
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
        bool autoRead = default(true);
        volatile int readSize @unit(B) = default(-1B);    // used only with autoRead==false 
        volatile double readDelay @unit(s) = default(-1s);    // used only with autoRead==false, negative delay means immediatelly send a read command, otherwise schedule timer with >= 0 delay for waiting to send a read command  
        @display("i=block/socket");
    gates:
        input trafficIn;
        output trafficOut;
        input socketIn;
        output socketOut;
}
