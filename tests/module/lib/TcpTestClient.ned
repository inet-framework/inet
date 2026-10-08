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
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
}

