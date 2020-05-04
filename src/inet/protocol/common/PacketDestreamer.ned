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

package inet.protocol.common;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IPacketFlow;

//
// This module takes packets streamed to its input and passes them to its output. 
//
simple PacketDestreamer extends PacketProcessorBase like IPacketFlow
{
    parameters:
        @class(PacketDestreamer);
        @display("i=block/cogwheel");
    gates:
        input in @labels(send,push,pull,stream);
        output out @labels(send,push,pull,pass);
}
