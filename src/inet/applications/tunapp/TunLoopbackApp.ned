//
// Copyright (C) 2015 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//TODO review
package inet.applications.tunapp;

import inet.applications.contract.IApp;

//
// Provides loopback functionality for a TUN interface by receiving packets,
// swapping source and destination addresses and ports, and sending them back
// through the same interface. Processes both network and transport layer
// headers to properly reverse packet direction. Useful for testing network
// configurations and protocols without requiring external connections.
//
simple TunLoopbackApp like IApp
{
    parameters:
        string interfaceTableModule;
        string tunInterface;
        @display("i=block/app");
    gates:
        input socketIn @labels(TunInterface/up);
        output socketOut @labels(TunInterface/down);
}
