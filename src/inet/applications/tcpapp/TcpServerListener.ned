//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

import inet.applications.contract.IApp;

//
// This module accepts/rejects TCP connections and creates TCP server connections
// as part of a generic TCP server application.
//
// Accepted TCP connections will inherit the autoRead setting of this module. To
// allow rate limiting on the connections, set autoRead=false here, and use
// corresponding parameters of the connection handling module (e.g. readSize and
// readDelay) to set a rate limit. This will allow TCP flow control to come into
// effect.
//
// @see ~TcpServerApp, ~TcpServerConnection
//
simple TcpServerListener like IApp
{
    parameters:
        string localAddress = default(""); // may be left empty ("")
        int localPort = default(1000); // port number to listen on
        string serverConnectionModuleType = default("inet.applications.tcpapp.TcpServerConnection"); // module type of "connection" module to launch on incoming connections
        bool autoRead = default(true); // defines the autoRead setting for connection handling modules
        @display("i=block/server");
    gates:
        input socketIn;
        output socketOut;
}
