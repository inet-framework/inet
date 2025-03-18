//
// Copyright (C) 2015 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.tunapp;

import inet.applications.contract.IApp;

//
// Creates a network tunnel between a TUN interface and a remote endpoint using
// either UDP or IPv4 protocols. Captures packets from the specified TUN
// interface and forwards them to the configured destination, while also
// receiving packets from the remote endpoint and injecting them into the TUN
// interface. Supports lifecycle operations for controlled startup and shutdown.
//
simple TunnelApp like IApp
{
    parameters:
        string interfaceTableModule;
        string interface;
        string protocol @enum("udp", "ipv4") = default("udp");
        string destinationAddress = default("");
        int destinationPort = default(-1);
        int localPort = default(-1);
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after the lifecycle stop operation finishes
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for the lifecycle stop operation
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
}
