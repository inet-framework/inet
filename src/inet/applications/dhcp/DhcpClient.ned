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
// Implements the DHCP client protocol. DHCP (Dynamic Host Configuration Protocol),
// described in RFC 2131, provides configuration parameters to Internet hosts.
// Requires UDP.
//
// @see ~DhcpServer, ~DhcpMessage
//
simple DhcpClient like IApp
{
    parameters:
        string interfaceTableModule; // path to the InterfaceTable module
        string routingTableModule; // path to the Ipv4RoutingTable module
        string interface = default(""); // which NIC should be configured (i.e. wlan0, eth1 etc.)
        double startTime @unit(s) = default(0s); // starting time of DHCP initialization
        @display("i=block/app2");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        @selfMessageKinds(inet::DhcpTimerType);
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}

