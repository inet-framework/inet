//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.dhcp;

import inet.applications.contract.IApp;

//
// Implements a standards-compliant DHCP client that automatically obtains network
// configuration from a DHCP server. Follows the Dynamic Host Configuration Protocol
// as specified in RFC 2131, enabling hosts to acquire IP addresses, subnet masks,
// default gateways, and other network parameters dynamically without manual configuration.
//
// The client progresses through the standard DHCP state machine (INIT, SELECTING,
// REQUESTING, BOUND) and handles lease renewal and rebinding according to the protocol
// specification. When started, it broadcasts DHCPDISCOVER messages to locate available
// servers, selects from received offers, requests the selected configuration, and
// applies the received parameters to the specified network interface. Supports
// lifecycle operations for controlled startup and shutdown.
//
// @see ~DhcpServer, ~DhcpMessage, ~UdpSocket
//
simple DhcpClient like IApp
{
    parameters:
        string interfaceTableModule; // Path to the InterfaceTable module
        string routingTableModule; // Path to the Ipv4RoutingTable module
        string interface = default(""); // Which NIC should be configured (i.e. wlan0, eth1 etc.)
        double startTime @unit(s) = default(0s); // Starting time of DHCP initialization
        @display("i=block/app2");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
        @selfMessageKinds(inet::DhcpTimerType);
    gates:
        input socketIn @labels(UdpCommand/up);
        output socketOut @labels(UdpCommand/down);
}
