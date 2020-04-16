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

package inet.queueing.common;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IActivePacketSink;

//
// This module connects multiple packet collectors to one packet provider. When
// a packet is popped by one of the connected packet collectors, then the demultiplexer
// simply pops a packet from the connected packet provider.
//
simple PacketDemultiplexer extends PacketProcessorBase like IActivePacketSink // TODO: , IPassivePacketSource // out gate is vector
{
    parameters:
        displayStringTextFormat = default("passed %p pk (%l)");
        @class(PacketDemultiplexer);
        @display("i=block/fork");
    gates:
        input in @labels(pop);
        output out[] @labels(pop);
}
