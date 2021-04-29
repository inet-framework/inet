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

//
// This module serves as a basis for gate scheduling configurator modules. It
// provides methods for derived modules to easily extract the input parameters
// from the network topology and also to carry out the actual configuration of
// the resulting schedule. The schedule is automatically configured in all of
// the ~PeriodicGate submodules of all queue submodules in the network interface
// MAC layer submodules. Besides, the application start offsets are also configured.
// This allows the derived modules to focus on the implementation of the actual
// scheduling algorithm.
//
simple GateSchedulingConfiguratorBase
{
    parameters:
        double gateCycleDuration @unit(s); // the globally used gate scheduling period in each PeriodicGate modules
        object configuration = default([]); // a vector of objects (e.g. [{...}, {...}]) where each object has the following fields: name, type, priority, application, source, destination, packetLength, packetInterval, maxLatency, pathFragments
                                            // here is an example: [{name: "s1", type: "unicast", application: "app[0]", priority: 0, source: "wheel*", destination: "hud", packetLength: 100B, packetInterval: 5ms, maxLatency: 100us}]
        @display("i=block/cogwheel");
}

