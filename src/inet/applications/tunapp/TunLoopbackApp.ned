//
// Copyright (C) 2015 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.tunapp;

import inet.common.SimpleModule;
import inet.applications.contract.IApp;

//
// Provides loopback functionality for a TUN interface by receiving packets,
// swapping source and destination addresses and ports, and sending them back
// through the same interface. Processes both network and transport layer
// headers to properly reverse packet direction. Useful for testing network
// configurations and protocols without requiring external connections.
//
// @see TunInterface
//
simple TunLoopbackApp extends SimpleModule like IApp
{
    parameters:
        @class(TunLoopbackApp);
        string interfaceTableModule;
        string tunInterface;
        @display("i=block/app");
    gates:
        input socketIn @labels(TunInterface/up);
        output socketOut @labels(TunInterface/down);
}

