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

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// This module decodes the stream of a packet by matching various tags on the
// packet against the mapping. It can check for network interface, source address,
// destination address, VLAN id, and PCP. If a match was found then the stream
// is attached to the packet in a ~StreamInd tag.
//
// @see ~StreamEncoder
//
simple StreamDecoder extends PacketFlowBase like IPacketFlow
{
    parameters:
        string interfaceTableModule; // relative module path of the interface table
        object mapping @mutable = default([]); // array of objects, where each object maps a network interface name pattern, a source address, a destination address, a VLAN id and a PCP value to a stream name, for example [{stream: "s1", interface: "eth0", destination: "dest1", pcp: 3}, ...]
        @class(StreamDecoder);
        @display("i=block/blackboard");
}
