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
        bool autoRead = default(true); // when false, the app never reads received data (fills the receive buffer -> zero window)
        double tOpen @unit(s);
        double tSend @unit(s);
        int sendBytes @unit(B);
        string sendScript;
        double tClose @unit(s);
        bool fastOpen = default(false); // TCP Fast Open (RFC 7413): request it on the first connection's active open
        bool active2 = default(false); // open a second, independent connection from this same app (and Tcp module) -- e.g. to prime a Fast Open cookie cache with a first connection, then exercise it with a second
        double tOpen2 @unit(s) = default(-1s); // -1s = no second connection
        int localPort2 = default(-1);
        double tSend2 @unit(s) = default(-1s);
        int sendBytes2 @unit(B) = default(0B);
        bool fastOpen2 = default(false); // TCP Fast Open (RFC 7413): request it on the second connection's active open, independently of fastOpen
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
}

