//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.inet;

import inet.node.base.ApplicationLayerNodeBase;

//
// A comprehensive model of an IPv4/IPv6 host that provides a complete network
// stack with configurable application, transport, network, and link layers. It
// supports multiple network interfaces, protocols, and applications that can be
// customized through parameters.
//
// @see ~WirelessHost, ~AdhocHost, ~Router
//
module StandardHost extends ApplicationLayerNodeBase
{
    parameters:
        @display("i=device/pc2");
        @figure[submodules];
        forwarding = default(false);  // Disable routing by default
        ipv4.arp.proxyArpInterfaces = default(""); // Proxy arp is disabled on hosts by default
        *.routingTableModule = default("^.ipv4.routingTable");
}

