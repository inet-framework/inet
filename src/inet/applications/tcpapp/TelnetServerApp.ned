//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

//
// Specialized TCP server application that handles telnet connections from
// multiple clients. Extends the generic TcpServerApp by configuring it to
// create TelnetServerConnection modules for each incoming connection.
//
// Designed to work with TelnetClientApp modules, this server listens for
// incoming connections and dynamically instantiates connection handlers
// that process client commands and generate appropriate responses. The
// server architecture supports concurrent connections from multiple clients,
// with each connection managed by a separate TelnetServerConnection instance
// that implements the telnet protocol behavior. Inherits all the connection
// management capabilities of TcpServerApp while providing telnet-specific
// functionality.
//
// @see ~TelnetClientApp, ~TelnetServerConnection, ~TelnetApp, ~TcpServerApp
//
module TelnetServerApp extends TcpServerApp
{
    parameters:
        listener.serverConnectionModuleType = default("TelnetServerConnection");
}
