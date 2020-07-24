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

package inet.queueing.filter;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// This module filters for packets matching the configured filter criteria.
// When the filter is pushed by a connected packet producer, it either pushes
// the packet to the connected packet consumer or drops it depending on the
// configured packet filter. Similarily, when the filter is pulled by a connected
// packet collector, it pulls packets from the connected packet provider until
// it finds one which matches the configured packet filter.
//
simple ContentBasedFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        string packetFilter = default("*"); // packet filter expressions (e.g. ping-*), matches all packets by default
        string packetDataFilter = default("*"); // packet data filter expression (e.g. inet::Ipv4Header and srcAddress = "10.0.0.1"), matches all packets by default
        @class(ContentBasedFilter);
}
