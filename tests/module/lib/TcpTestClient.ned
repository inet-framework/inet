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
        bool active3 = default(false); // open a third, independent connection from this same app (and Tcp module)
        double tOpen3 @unit(s) = default(-1s); // -1s = no third connection
        int localPort3 = default(-1);
        double tSend3 @unit(s) = default(-1s);
        int sendBytes3 @unit(B) = default(0B);
        bool fastOpen3 = default(false); // TCP Fast Open (RFC 7413): request it on the third connection's active open, independently of fastOpen/fastOpen2
        string statusRequestScript = default(""); // comma-separated list of times ("1.5,3.0") at which to issue a TCP_C_STATUS query on socket (the first connection) and log the reply
        bool enableTimestamping = default(false); // Workstream H3 (SO_TIMESTAMPING): if true, calls socket.setTimestamping(true) right after bind(), logging TcpRxTimestampInd's timestamp on every arriving TCP_I_DATA packet
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
}

