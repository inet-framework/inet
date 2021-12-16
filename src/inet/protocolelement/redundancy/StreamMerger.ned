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

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// This module merges the packets of the same stream by removing duplicates. In
// addition it also replaces the stream name on the packet based on the mapping
// parameter. For merging it maintains a separate finite buffer per stream with
// the last seen sequence numbers.
//
simple StreamMerger extends PacketFilterBase like IPacketFilter
{
    parameters:
        object mapping @mutable = default({}); // map of input streams to output streams, empty string means no stream, for example {s0: "s1", ...}
        int bufferSize = default(10); // window size for sequence numbers per stream
        @class(StreamMerger);
        @display("i=block/join");
}
