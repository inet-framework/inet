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

package inet.protocolelement.redundancy;

import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketFlow;

module StreamIdentifierLayer like IProtocolLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        streamIdentifier: <default("StreamIdentifier")> like IPacketFlow if typename != "" {
            parameters:
                @display("p=200,150;i=block/blackboard");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> streamIdentifier.in;
        streamIdentifier.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> upperLayerOut;
}
