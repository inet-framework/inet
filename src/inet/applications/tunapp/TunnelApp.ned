//
// Copyright (C) 2015 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.tunapp;

import inet.common.SimpleModule;
import inet.applications.contract.IApp;

simple TunnelApp extends SimpleModule like IApp
{
    parameters:
        @class(TunnelApp);
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
