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

package inet.linklayer.vlan;

import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketFlow;

module VlanPolicyLayer like IProtocolLayer
{
    parameters:
        @display("i=block/layer");
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        outboundMapper: <default("VlanReqMapper")> like IPacketFlow {
            @display("p=200,150");
        }
        outboundFilter: <default("VlanReqFilter")> like IPacketFilter {
            @display("p=200,300");
        }
        inboundMapper: <default("")> like IPacketFlow {
            @display("p=500,150");
        }
        inboundFilter: <default("VlanIndFilter")> like IPacketFilter {
            @display("p=500,300");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> outboundMapper.in;
        outboundMapper.out --> outboundFilter.in;
        outboundFilter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> inboundFilter.in;
        inboundFilter.out --> inboundMapper.in;
        inboundMapper.out --> { @display("m=n"); } --> upperLayerOut;
}
