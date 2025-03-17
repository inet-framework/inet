//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

import inet.applications.contract.IApp;
import inet.common.MessageDispatcher;

//
// Implements a modular, composite TCP server application with a flexible architecture
// for handling multiple client connections. Consists of a listener component that
// manages connection acceptance and a dynamic array of connection handler modules
// that process individual client sessions. This design separates connection management
// from application-specific processing logic.
//
// The architecture uses a message dispatcher to route messages between the socket
// interface, the listener, and the connection handlers. When a new connection arrives,
// the listener creates a new connection module in the connection[] vector to handle
// that specific client. Each connection module can implement custom application logic
// while sharing the server's socket interface.
//
// This modular approach enables the creation of various TCP server applications by
// simply configuring different connection handler module types without modifying the
// server framework itself. The server can be used for implementing HTTP servers,
// FTP servers, custom application protocols, or any service requiring TCP connection
// handling.
//
// @see ~TcpClientApp, ~TcpServerListener, ~TcpServerConnection, ~MessageDispatcher
//
module TcpServerApp like IApp
{
    parameters:
        string interfaceTableModule; // Relative module path of the interface table
        *.interfaceTableModule = default(this.interfaceTableModule);
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
    submodules:
        listener: TcpServerListener {
            parameters:
                @display("p=100,100");
        }
        connection[0]: <default(listener.serverConnectionModuleType)> like IApp {
            parameters:
                @display("p=200,100,row,50");
        }
        ta: MessageDispatcher {
            parameters:
                @display("p=100,200");
        }
    connections:
        ta.out++ --> listener.socketIn;
        listener.socketOut --> ta.in++;
        ta.out++ --> { @display("m=s"); } --> socketOut;
        socketIn --> { @display("m=s"); } --> ta.in++;
}
