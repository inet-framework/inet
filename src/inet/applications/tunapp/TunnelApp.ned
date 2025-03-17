//
// Copyright (C) 2015 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.tunapp;

import inet.applications.contract.IApp;

//
// Implements a network tunneling application that encapsulates packets from one
// interface and transmits them through another protocol. Enables the creation of
// virtual network tunnels that can transport packets across networks that might
// not natively support the encapsulated protocol, or to provide secure communication
// channels through untrusted networks.
//
// Supports multiple tunneling protocols including UDP and IPv4 encapsulation.
// The application captures packets from a specified interface, encapsulates them
// according to the selected protocol, and transmits them to a configured destination.
// At the receiving end, a corresponding TunnelApp instance decapsulates the packets
// and injects them into the local network stack. This mechanism allows for extending
// networks across incompatible infrastructure or implementing VPN-like functionality.
//
// @see ~TunLoopbackApp, ~TunInterface
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
