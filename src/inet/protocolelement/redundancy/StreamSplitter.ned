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

import inet.queueing.base.PacketPusherBase;
import inet.queueing.contract.IPacketPusher;

simple StreamSplitter extends PacketPusherBase like IPacketPusher
{
    parameters:
        object mapping = default({}); // map from input stream names to array of output stream names, empty string means no stream
        @class(StreamSplitter);
        @display("i=block/fork");
}
