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
// This module identifies the stream of a packet using packet filter expressions.
// It can check for arbitrary packet data and assign and if a match was found then
// the stream is attached to the packet in a ~StreamReq tag.
//
simple StreamIdentifier extends PacketFlowBase like IPacketFlow
{
    parameters:
        object mapping @mutable = default([]); // array of objects, where each object specifies a stream name, a packetFilter and a dataFilter, for example [{stream: "s0", packetFilter: "*", dataFilter: "*"}, ...]
        @class(StreamIdentifier);
        @display("i=block/blackboard");
}
