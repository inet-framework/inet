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
// @see ~TcpServerApp, ~TcpServerConnection
//
simple TcpServerListener like IApp
{
    parameters:
        string localAddress = default(""); // may be left empty ("")
        int localPort = default(1000); // port number to listen on
        string serverConnectionModuleType = default("inet.applications.tcpapp.TcpServerConnection"); // module type of "connection" module to launch on incoming connections
        @display("i=block/server");
    gates:
        input socketIn;
        output socketOut;
}
