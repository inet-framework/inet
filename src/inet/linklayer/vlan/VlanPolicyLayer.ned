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

package inet.linklayer.vlan;

import inet.linklayer.ethernet.redundancy.IProtocolLayer;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketFlow;

module VlanPolicyLayer like IProtocolLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        outboundMapper: <default("VlanReqMapper")> like IPacketFlow {
            parameters:
                @display("p=200,100");
        }
        outboundFilter: <default("VlanReqFilter")> like IPacketFilter {
            parameters:
                @display("p=200,200");
        }
        inboundMapper: <default("OmittedPacketFlow")> like IPacketFlow {
            parameters:
                @display("p=500,100");
        }
        inboundFilter: <default("VlanIndFilter")> like IPacketFilter {
            parameters:
                @display("p=500,200");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> outboundMapper.in;
        outboundMapper.out --> outboundFilter.in;
        outboundFilter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> inboundFilter.in;
        inboundFilter.out --> inboundMapper.in;
        inboundMapper.out --> { @display("m=n"); } --> upperLayerOut;
}
