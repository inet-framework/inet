//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator;

import inet.networklayer.configurator.contract.INetworkConfigurator;

//
// This module provides Time-Sensitive Networking (TSN) static stream redundancy
// configuration. The module automatically configures all the necessary modules
// related to stream splitting, stream merging and stream filtering in all network
// nodes. The configuration parameter specifies the streams with a set of path
// fragments.
//
simple StreamRedundancyConfigurator like INetworkConfigurator
{
    parameters:
        int minVlanId = default(0); // lowest available VLAN ID
        int maxVlanId = default(4095); // highest available VLAN ID
        object configuration @mutable = default([]); // a vector of objects (e.g. [{...}, {...}]) where each object has the following fields: name, source, destination, packetFilter, paths
                                                     // here is an example: [{name: "S1", packetFilter: "*", source: "source", destination: "destination", trees: [[["s1", "s2a", "s3a"]], [["s1", "s2b", "s3b"]], [["s1", "s2a", "s2b", "s3b"]], [["s1", "s2b", "s2a", "s3a"]]]}]
        @display("i=block/cogwheel");
}

