//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

//
// This module provides TCP server socket handling for generic TCP applications.
//
// @see ~TcpServerConnection, ~TcpClientSocketIo
//
simple TcpServerSocketIo
{
    parameters:
        @display("i=block/socket");
        volatile int readSize @unit(B) = default(-1B);    // used only with usingTcpWithRead==true 
        volatile double readDelay @unit(s) = default(-1s);    // used only with usingTcpWithRead==true, negative delay means immediatelly send a read command, otherwise schedule timer with >= 0 delay for waiting to send a read command  
    gates:
        input trafficIn;
        output trafficOut;
        input socketIn;
        output socketOut;
}
