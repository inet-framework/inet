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

package inet.linklayer.configurator.gatescheduling.base;

simple GateSchedulingConfiguratorBase
{
    parameters:
        double gateCycleDuration @unit(s); // the globally used gating schedule period in each PeriodicGate modules
        object configuration = default([]); // a vector of objects (e.g. [{...}, {...}]) where each object has the following fields: type, priority, source, destination, packetLength, packetInterval, maxLatency, pathFragments
                                            // here is an example: [{type: "unicast", priority: 0, source: "wheel*", destination: "hud", packetLength: 100B, packetInterval: 5ms, maxLatency: 100us}]
        @display("i=block/cogwheel");
}

