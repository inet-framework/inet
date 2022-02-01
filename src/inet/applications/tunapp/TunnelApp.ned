//
// Copyright (C) 2015 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.tunapp;

import inet.applications.contract.IApp;

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
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
}
