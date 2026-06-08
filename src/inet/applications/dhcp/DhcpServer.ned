//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.dhcp;

import inet.common.SimpleModule;
import inet.applications.contract.IApp;

//
// Implements the DHCP server protocol. DHCP (Dynamic Host Configuration Protocol),
// described in RFC 2131, provides configuration parameters to Internet hosts.
// Requires UDP.
//
// @see ~DhcpClient, ~DhcpMessage
//
simple DhcpServer extends SimpleModule like IApp
{
    parameters:
        @class(DhcpServer);
        string interfaceTableModule; // The path to the InterfaceTable module
        string interface = default(""); // Interface to listen on; can be left empty if there is only one non-loopback interface
        int numReservedAddresses; // Number of addresses to skip at the start of the network's address range
        int maxNumClients; // Maximum number of clients (IPs) allowed to be leased
        string gateway = default(""); // Gateway to assign, if empty use the interface address
        int leaseTime @unit(s); // Lease time in seconds
        double offerHoldTime @unit(s) = default(120s); // How long an offered (but not yet acknowledged) address is reserved (RFC 2131 SHOULD timer T2-equivalent for offers)
        double declineHoldTime @unit(s) = default(60s); // How long an address remains unavailable after a DHCPDECLINE
        double startTime @unit(s) = default(0s); // Application start time
        @display("i=block/app2");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
    gates:
        input socketIn @labels(UdpCommand/up);
        output socketOut @labels(UdpCommand/down);
}

