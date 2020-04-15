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

package inet.queueing.base;

//
// This is a base module for various packet buffer modules which maintains a few
// statistics.
//
// Available display string format directives:
// - %p - number of packet
// - %l - total length
// - %a - number of added packets
// - %r - number of removed packets
// - %d - number of dropped packets
//
// @see ~IPacketBuffer
//
simple PacketBufferBase extends PacketProcessorBase
{
    parameters:
        displayStringTextFormat = default("contains %p pk (%l)\nadded %a removed %r dropped %d"); // determines display string text above the submodule
        @display("i=block/buffer");
    gates:
        input in @loose;
        output out @loose;
}
