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
// This is a base module for various packet source modules. Packets are created
// randomly with regard to packet length and packet data. The representation of
// packet data can also be configured.
//
// @see ~IPacketSource
//
simple PacketSourceBase extends PacketProcessorBase
{
    parameters:
        string packetNameFormat = default("%n-%c"); // %n module name, %c packet number, %l packet length, %d packet data, %t simulation time, %e event number
        string packetRepresentation @units("bitCount","bits","byteCount","bytes","applicationPacket") = default("byteCount"); // determines the chunk of the packet data
        volatile int packetLength @unit(b);
        volatile int packetData = default(-1);
        displayStringTextFormat = default("created %p pk (%l)");
        @class(PacketSourceBase);
        @display("i=block/source");
}
