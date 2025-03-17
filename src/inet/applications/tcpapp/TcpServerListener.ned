//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

import inet.applications.contract.IApp;

//
// TCP server component that listens for incoming connections and dynamically creates
// handler modules for each accepted connection. Designed as a building block for
// implementing scalable server applications that can handle multiple concurrent
// client connections.
//
// Acts as a connection factory that binds to a specified local address and port,
// then spawns configurable connection handler modules when clients connect. The
// newly created handler modules become siblings of this module in the simulation
// hierarchy. Provides flow control capabilities through the autoRead parameter,
// which when set to false allows connection handlers to implement rate limiting
// through parameters like readSize and readDelay, enabling TCP's built-in flow
// control mechanisms to regulate traffic.
//
// @see ~TcpServerApp, ~TcpServerConnection, ~TcpSocket
//
simple TcpServerListener
{
    parameters:
        string localAddress = default(""); // May be left empty ("")
        int localPort = default(1000); // Port number to listen on
        string serverConnectionModuleType = default("inet.applications.tcpapp.TcpServerConnection"); // Module type of "connection" module to launch on incoming connections
        bool autoRead = default(true); // Defines the autoRead setting for connection handling modules
        @display("i=block/server");
    gates:
        input socketIn;
        output socketOut;
}
