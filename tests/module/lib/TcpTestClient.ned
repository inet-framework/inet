//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.test.moduletest.lib;

import inet.applications.contract.IApp;

//
// \TCP client application for testing the TCP model.
//
simple TcpTestClient like IApp
{
    parameters:
        string localAddress;
        int localPort;
        bool active;
        string connectAddress;
        int connectPort;
        string dataTransferMode @enum("bytecount","object","bytestream") = default("bytecount");
        double tOpen @unit(s);
        double tSend @unit(s);
        int sendBytes @unit(B);
        string sendScript;
        double tClose @unit(s);
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
}

