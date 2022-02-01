//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

//
// This server application accepts and creates telnet server connections.
//
// @see ~TelnetClientApp, ~TelnetServerConnection
//
module TelnetServerApp extends TcpServerApp
{
    parameters:
        listener.serverConnectionModuleType = default("TelnetServerConnection");
}
