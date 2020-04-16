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

package inet.queueing.marker;

import inet.queueing.base.PacketLabelerBase;
import inet.queueing.contract.IPacketMarker;

//
// This module attaches various request labels to packets.
//
simple ContentBasedLabeler extends PacketLabelerBase like IPacketMarker
{
    parameters:
        string packetFilters = default("*"); // semicolon separated packet filter expressions (e.g. ping-*), matches all packets by default
        string packetDataFilters = default("*"); // semicolon separated packet data filter expression (e.g. inet::Ipv4Header and srcAddress = "10.0.0.1"), matches all packets by default
        @class(ContentBasedLabeler);
}
