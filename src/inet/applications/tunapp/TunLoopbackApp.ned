//
// Copyright (C) 2015 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.tunapp;

import inet.applications.contract.IApp;

//
// Implements a simple loopback application for TUN interfaces that forwards packets
// received from a TUN interface back to the same interface. Provides a mechanism
// for testing TUN interface functionality by creating a feedback loop where outgoing
// packets are immediately returned as incoming packets.
//
// The application connects to a specified TUN interface and processes all packets
// that arrive from the network stack through this interface. Each received packet
// is immediately sent back to the same interface, allowing the testing of packet
// transmission and reception through the TUN interface without requiring external
// network communication.
//
// @see ~TunnelApp, ~TunInterface
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
