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
import inet.queueing.contract.IPacketFlow;

//
// This module combines the interface MAC address learning from incoming packets
// with the outgoing interface selection for outgoing packets into a single layer.
//
module InterfaceRelayLayer like IProtocolLayer
{
    parameters:
        string macTableModule;
        string interfaceTableModule;
        *.macTableModule = default(absPath(this.macTableModule));
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        learner: <default("RelayInterfaceLearner")> like IPacketFlow {
            @display("p=200,150");
        }
        selector: <default("RelayInterfaceSelector")> like IPacketFlow {
            @display("p=500,150");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> selector.in;
        selector.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> learner.in;
        learner.out --> { @display("m=n"); } --> upperLayerOut;
}
