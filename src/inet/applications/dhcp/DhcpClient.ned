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
// Implements the DHCP client protocol. DHCP (Dynamic Host Configuration Protocol),
// described in RFC 2131, provides configuration parameters to Internet hosts.
// Requires UDP.
//
// @see ~DhcpServer, ~DhcpMessage
//
simple DhcpClient extends SimpleModule like IApp
{
    parameters:
        @class(DhcpClient);
        string interfaceTableModule; // Path to the InterfaceTable module
        string routingTableModule; // Path to the Ipv4RoutingTable module
        string arpModule = default("^.ipv4.arp"); // Path to the ARP module, used to probe the granted address
        // RFC 2131 section 4.4.1 asks the client to check that the granted address is not
        // already in use before it takes it, and RFC 5227 section 2.1.1 makes that check an
        // ARP probe. This is how long the client waits for an answer; 0s turns the check off
        // and takes the address at once.
        double probeWait @unit(s) = default(1s);
        string interface = default(""); // Which NIC should be configured (i.e. wlan0, eth1 etc.)
        double startTime @unit(s) = default(0s); // Starting time of DHCP initialization
        displayStringTextFormat = default("{clientStateName}");
        @display("i=block/app2");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
        @selfMessageKinds(inet::DhcpTimerType);
    gates:
        input socketIn @labels(UdpCommand/up);
        output socketOut @labels(UdpCommand/down);
}

