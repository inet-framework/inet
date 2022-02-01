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
    gates:
        input trafficIn;
        output trafficOut;
        input socketIn;
        output socketOut;
}
