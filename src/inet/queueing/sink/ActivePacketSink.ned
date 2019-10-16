//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.queueing.sink;

import inet.queueing.base.PacketSinkBase;
import inet.queueing.contract.IActivePacketSink;

//
// This module is an active packet sink which pops packets from the connected
// packet provider. All popped packets are counted and deleted.
//
simple ActivePacketSink extends PacketSinkBase like IActivePacketSink
{
    parameters:
        volatile double collectionInterval @unit(s); // elapsed time between subsequent packets popped from the connected packet provider
        @class(ActivePacketSink);
    gates:
        input in @labels(pop);
}

