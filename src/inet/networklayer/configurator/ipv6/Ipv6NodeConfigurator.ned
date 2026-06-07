//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.configurator.ipv6;

import inet.common.SimpleModule;
import inet.networklayer.configurator.contract.IIpv6NodeConfigurator;

//
// Configures the containing network node using information provided by
// the network's global configurator module ~Ipv6NetworkConfigurator.
//
// ~Ipv6NetworkConfigurator only computes and remembers configuration data
// (assigned IPv6 addresses, static routes, etc), but doesn't configure the
// nodes' routing tables and interfaces accordingly. The actual configuration
// is carried out by this module, at network initialization time and also when
// the node restarts (i.e. after a shutdown or a crash.)
//
// This module is needed because of failure/restart scenarios:
// ~Ipv6NetworkConfigurator would not be able to configure the node after
// a restart, because being a global module, it doesn't know about node
// lifecycle events.
//
simple Ipv6NodeConfigurator extends SimpleModule like IIpv6NodeConfigurator
{
    parameters:
        @class(Ipv6NodeConfigurator);
        string interfaceTableModule;
        string routingTableModule;
        string networkConfiguratorModule = default(""); // The absolute path to the Ipv6NetworkConfigurator; use "" if there is no configurator
        bool configureRoutingTable = default(true);
        @display("i=block/cogwheel");
}


