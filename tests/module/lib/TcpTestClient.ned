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
        bool fork = default(false); // passive open: true = listen() (a new connection for each SYN), false = listenOnce()
        string connectAddress;
        int connectPort;
        string dataTransferMode @enum("bytecount","object","bytestream") = default("bytecount");
        double tOpen @unit(s);
        double tSend @unit(s);
        int sendBytes @unit(B);
        string sendScript;
        double tClose @unit(s);
        bool autoRead = default(true); // when false, the app reads received data only by the READ commands of readScript (fills the receive buffer -> zero window)
        string readScript = default(""); // with autoRead=false: "time bytes; time bytes; ..." -- at each time, issue a READ of at most that many bytes
        string statusRequestScript = default(""); // comma-separated list of times ("1.5,3.0") at which to issue a TCP_C_STATUS query on the socket; the reply is logged as a "STATUS:" line
        bool logIndications = default(false); // log each indication of TCP (with the number of bytes of its data) in the "testing" category
        bool fastOpen = default(false); // active open: request TCP Fast Open (RFC 7413) for the connection
        double tOpen2 @unit(s) = default(-1s); // the open of a second connection of this application (and Tcp module), for example to use the Fast Open cookie that the first connection got; -1s: no second connection
        bool active2 = default(false); // the second connection: active open (to connectAddress:connectPort), or passive open (listenOnce())
        int localPort2 = default(-1); // the local port of the second connection
        double tSend2 @unit(s) = default(-1s); // the time of the SEND of the second connection
        int sendBytes2 @unit(B) = default(0B); // the number of bytes of the SEND of the second connection; 0B: no SEND
        bool fastOpen2 = default(false); // the second connection: request TCP Fast Open
        double tOpen3 @unit(s) = default(-1s); // a third connection, as the second one
        bool active3 = default(false);
        int localPort3 = default(-1);
        double tSend3 @unit(s) = default(-1s);
        int sendBytes3 @unit(B) = default(0B);
        bool fastOpen3 = default(false);
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
}

