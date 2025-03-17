//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.contract;

//
// Defines the standard interface for all application modules in the INET Framework.
// Serves as a contract that application implementations must fulfill, providing
// a consistent connection point between applications and underlying transport
// protocols. The interface specifies input and output gates that allow applications
// to exchange messages with transport layer protocols through standardized socket
// APIs.
//
// By implementing this interface, application modules can be easily interchanged
// in network simulations while maintaining compatibility with the transport layer.
// This modularity enables flexible network architecture design and simplifies the
// creation of diverse simulation scenarios with different application behaviors.
//
// @see ~UdpBasicApp, ~TcpBasicClientApp, ~UdpVideoStreamServer, ~PingApp
//
moduleinterface IApp
{
    parameters:
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
}
