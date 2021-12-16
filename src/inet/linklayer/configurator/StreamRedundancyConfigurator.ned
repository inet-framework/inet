//
// Copyright (C) 2020 OpenSim Ltd.
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
// This module provides Time-Sensitive Networking (TSN) static stream redundancy
// configuration. The module automaticall configures all the necessary modules
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
                                                     // here is an example: [{name: "S1", packetFilter: "*", source: "source", destination: "destination", paths: [["s1", "s2a", "s3a"], ["s1", "s2b", "s3b"], ["s1", "s2a", "s2b", "s3b"], ["s1", "s2b", "s2a", "s3a"]]}]
        @display("i=block/cogwheel");
}

