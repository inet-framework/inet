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
        @display("i=block/socket");
    gates:
        input trafficIn;
        output trafficOut;
        input socketIn;
        output socketOut;
}
