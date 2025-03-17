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
// Implements a standards-compliant DHCP server that dynamically assigns network
// configuration to client hosts. Follows the Dynamic Host Configuration Protocol
// as specified in RFC 2131, providing automatic allocation of IP addresses, subnet
// masks, default gateways, and other network parameters to requesting clients.
//
// The server maintains an address pool and lease database, tracking which addresses
// are assigned to which clients and for how long. When receiving DHCPDISCOVER messages,
// it offers available addresses from its pool; when receiving DHCPREQUEST messages,
// it confirms the allocation and records the lease. The server handles address reuse
// after lease expiration and supports various configuration options including custom
// lease times and gateway specification. Designed to work with standard DHCP clients
// and supports lifecycle operations for controlled startup and shutdown.
//
// @see ~DhcpClient, ~DhcpMessage, ~UdpSocket
//
simple DhcpServer like IApp
{
    parameters:
        string interfaceTableModule; // The path to the InterfaceTable module
        string interface = default(""); // Interface to listen on; can be left empty if there is only one non-loopback interface
        int numReservedAddresses; // Number of addresses to skip at the start of the network's address range
        int maxNumClients; // Maximum number of clients (IPs) allowed to be leased
        string gateway = default(""); // Gateway to assign, if empty use the interface address
        int leaseTime @unit(s); // Lease time in seconds
        double startTime @unit(s) = default(0s); // Application start time
        @display("i=block/app2");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
    gates:
        input socketIn @labels(UdpCommand/up);
        output socketOut @labels(UdpCommand/down);
}
