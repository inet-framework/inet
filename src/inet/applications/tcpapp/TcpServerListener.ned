//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

import inet.applications.contract.IApp;

//
// This module opens a TCP socket to listen on a port, accepts incoming
// connections, and dynamically creates modules to handle them. The type of
// modules to be created can be specified in a parameter. The new modules
// will be appended to the connection[] submodule array of the parent of
// this module, i.e. they will be siblings of this module.
//
// Accepted TCP connections will inherit the autoRead setting of this module. To
// allow rate limiting on the connections, set autoRead=false here, and use
// corresponding parameters of the connection handling module (e.g. readSize and
// readDelay) to set a rate limit. This will allow TCP flow control to come into
// effect.
//
// @see ~TcpServerApp, ~TcpServerConnection
//
simple TcpServerListener
{
    parameters:
        string localAddress = default(""); // may be left empty ("")
        int localPort = default(1000); // port number to listen on
        string serverConnectionModuleType = default("inet.applications.tcpapp.TcpServerConnection"); // module type of "connection" module to launch on incoming connections
        bool autoRead = default(true); // defines the autoRead setting for connection handling modules
        @display("i=block/server");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
    gates:
        input socketIn;
        output socketOut;
}
