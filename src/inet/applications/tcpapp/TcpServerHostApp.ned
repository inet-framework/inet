//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

import inet.applications.contract.IApp;

//
// This module hosts TCP-based server applications. It dynamically creates
// and launches a new "thread" object for each incoming connection.
//
// Server threads should be subclassed from the TcpServerThreadBase
// C++ class, registered in the C++ code using the Define_Module() macro,
// and the module type should be specified in the serverThreadModuleType
// parameter of ~TcpServerHostApp. The thread object will receive events
// via a callback interface (methods like established(), dataArrived(),
// peerClosed(), timerExpired()), and can send packets via TcpSocket's send()
// method.
//
// IMPORTANT: Before you try to use this module, make sure you actually need it!
// In most cases, ~TcpGenericServerApp and ~GenericAppMsg will be completely
// enough, and they are a lot easier to handle. You'll want to subclass your
// client from TCPGenericCliAppBase then; check ~TelnetApp and ~TcpBasicClientApp
// for examples.
//
// Compatible with both ~Ipv4 and ~Ipv6.
//
simple TcpServerHostApp like IApp
{
    parameters:
        string localAddress = default(""); // may be left empty ("")
        int localPort = default(1000); // port number to listen on
        string serverThreadModuleType; // module type of "thread" module to launch on incoming connections
        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
    gates:
        input socketIn @labels(TcpCommand/up);
        output socketOut @labels(TcpCommand/down);
}

