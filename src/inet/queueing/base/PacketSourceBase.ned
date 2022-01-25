//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

package inet.queueing.base;

//
// This is a base module for various packet source modules. Packets are created
// randomly with regard to packet length and packet data. The representation of
// packet data can also be configured.
//
// Available packet name format directives:
// source, application, network node
// display name or full name, full path
//
//  - %a automatic application display name or application full name or module display name or module full name
//  - %n module display name or module full name
//  - %m application display name or application full name
//  - %M network node display name or network node full name
//  - %p module full path
//  - %h application full path
//  - %H network node full path
//  - %c packet number
//  - %l packet length
//  - %d packet data
//  - %t simulation time
//  - %e event number
//
// @see ~IPacketSource
//
simple PacketSourceBase extends PacketProcessorBase
{
    parameters:
        string packetNameFormat = default("%a-%c"); // see directives in module documentation
        string packetRepresentation @units("bitCount","bits","byteCount","bytes","applicationPacket") = default("byteCount"); // determines the chunk of the packet data
        volatile int packetLength @unit(b);
        volatile int packetData = default(-1);
        bool attachCreationTimeTag = default(true);
        bool attachIdentityTag = default(true);
        bool attachDirectionTag = default(true);
        displayStringTextFormat = default("created %p pk (%l)");
        @class(PacketSourceBase);
        @display("i=block/source");
        @signal[packetCreated](type=inet::Packet);
}
