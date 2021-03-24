//
// Copyright (C) 2021 OpenSim Ltd.
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

package inet.linklayer.ethernet.common;

import inet.protocolelement.contract.IProtocolLayer;

module RelayLayer like IProtocolLayer
{
    parameters:
        *.macTableModule = default("^.^.macTable");
        *.interfaceTableModule = default("^.^.interfaceTable");
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        learner: RelayInterfaceLearner {
            @display("p=100,100");
        }
        selector: RelayInterfaceSelector {
            @display("p=300,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> selector.in;
        selector.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> learner.in;
        learner.out --> { @display("m=n"); } --> upperLayerOut;
}
