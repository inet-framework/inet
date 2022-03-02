//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator;

import inet.networklayer.configurator.contract.INetworkConfigurator;

//
// This module provides Time-Sensitive Networking (TSN) configuration using other
// configurators. One is used to provie the stream redundancy (stream splitting
// and stream merging) configuration, the other one ise used to provide the gate
// scheduling configuration.
//
// The key parameters are the linkFailureProtection and nodeFailureProtection
// fields of the individual configuration entries. Both of these take a number
// and a set of links or nodes respectively. The former specifies the number of
// failed links or nodes from the given set against which the stream must be
// protected.
//
simple FailureProtectionConfigurator like INetworkConfigurator
{
    parameters:
        string streamRedundancyConfiguratorModule = default("streamRedundancyConfigurator");
        string gateScheduleConfiguratorModule = default("gateScheduleConfigurator");
        object configuration = default([]); // a vector of objects (e.g. [{...}, {...}]) where each object has the following fields: name, type, application, source, destination, PCP, gate index, packetFilter, packetLength, packetInterval, maxLatency, linkFailureProtection, nodeFailureProtection, maxRedundancy
                                            // here is an example: [{name: "s1", application: "app[0]", source: "source", destination: "destination", pcp: 0, gateIndex: 0, packetFilter: "*", packetLength: 200B, packetInterval: 500us, maxLatency: 100us, linkFailureProtection: [{any: 1, of: "*"}], maxRedundancy: 2}]
        @display("i=block/cogwheel");
}

