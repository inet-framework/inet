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
import inet.queueing.contract.IPassivePacketSink;

//
// This module connects one packet producer to one packet consumer. It can be
// pushed with packets from the connected packet producer. When this happens,
// a separate copy of the packet is pushed to each output gate.
//
simple PacketCloner extends PacketProcessorBase like IPassivePacketSink // IActivePacketSource // TODO: out gate is vector 
{
    parameters:
        @class(PacketCloner);
        @display("i=block/star");
    gates:
        input in @labels(push);
        output out[] @labels(push);
}
