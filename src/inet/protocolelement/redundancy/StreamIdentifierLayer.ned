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

package inet.protocolelement.redundancy;

import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketFlow;

//
// This module wraps a stream identifier into a protocol layer so that it can
// be used in a layered compound module.
//
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
        identifier: <default("StreamIdentifier")> like IPacketFlow if typename != "" {
            parameters:
                @display("p=200,150;i=block/blackboard");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> identifier.in;
        identifier.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> upperLayerOut;
}
