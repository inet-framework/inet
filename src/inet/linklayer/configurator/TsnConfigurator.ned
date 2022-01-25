//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
simple TsnConfigurator like INetworkConfigurator
{
    parameters:
        string streamRedundancyConfiguratorModule = default("streamRedundancyConfigurator");
        string gateSchedulingConfiguratorModule = default("gateSchedulingConfigurator");
        object configuration = default([]); // a vector of objects (e.g. [{...}, {...}]) where each object has the following fields: name, type, application, source, destination, priority, packetFilter, packetLength, packetInterval, maxLatency, linkFailureProtection, nodeFailureProtection, maxRedundancy
                                            // here is an example: [{name: "s1", application: "app[0]", source: "source", destination: "destination", priority: 0, packetFilter: "*", packetLength: 200B, packetInterval: 500us, maxLatency: 100us, linkFailureProtection: [{any: 1, of: "*"}], maxRedundancy: 2}]
        @display("i=block/cogwheel");
}

