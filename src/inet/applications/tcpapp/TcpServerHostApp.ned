//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

import inet.applications.contract.IApp;

//
// Provides a framework for implementing TCP server applications that can handle
// multiple concurrent client connections. Functions as a connection manager that
// listens for incoming TCP connections and creates a separate "thread" module
// instance for each connection, allowing servers to maintain independent state
// for each client and process multiple connections simultaneously.
//
// The actual server behavior is implemented in thread modules that must be derived
// from the TcpServerThreadBase C++ class. These thread modules receive connection
// events through a callback interface (established(), dataArrived(), peerClosed(),
// timerExpired()) and can send data using the TcpSocket's send() method. The specific
// thread module type is specified through the serverThreadModuleType parameter.
//
// This architecture enables the implementation of various server applications like
// echo servers, file transfer servers, and custom application protocols by simply
// creating different thread module implementations while reusing the connection
// management functionality.
//
// @see ~TcpEchoApp, ~TcpGenericServerApp, ~TcpServerThreadBase, ~TcpSocket
//
simple TcpServerHostApp like IApp
{
    parameters:
        string localAddress = default(""); // May be left empty ("")
        int localPort = default(1000); // Port number to listen on
        string serverThreadModuleType; // Module type of "thread" module to launch on incoming connections
        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
    gates:
        input socketIn @labels(TcpCommand/up);
        output socketOut @labels(TcpCommand/down);
}
