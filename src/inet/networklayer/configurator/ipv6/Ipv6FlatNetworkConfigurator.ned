//
// Copyright (C) 2004 Eric Wu
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.configurator.ipv6;

import inet.common.SimpleModule;
import inet.networklayer.configurator.contract.IL3NetworkConfigurator;

//
// Configures IPv6 addresses and routing tables for a "flat" network,
// "flat" meaning that all hosts and routers will have the same
// network address and will only differ in the host part.
//
//# FIXME add documentation!
//
// @see ~Ipv4FlatNetworkConfigurator
//
simple Ipv6FlatNetworkConfigurator extends SimpleModule like IL3NetworkConfigurator
{
    parameters:
        @class(Ipv6FlatNetworkConfigurator);
        @display("i=block/cogwheel");
        @labels(node);
}

