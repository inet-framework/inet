//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.networklayer.configurator.contract;

//
// Module interface for IPv4 node configurators that apply configuration data
// (IP addresses, static routes, etc.) to a node's interfaces and routing table.
// Node configurators work with network configurators but operate at the node
// level, handling node-specific configuration including initialization and
// reconfiguration after node restarts.
//
moduleinterface IIpv4NodeConfigurator
{
    parameters:
        @display("i=block/cogwheel");
}
