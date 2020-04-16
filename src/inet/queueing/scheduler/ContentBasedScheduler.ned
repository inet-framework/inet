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

package inet.queueing.scheduler;

import inet.queueing.base.PacketSchedulerBase;
import inet.queueing.contract.IPacketScheduler;

//
// This module connects one packet collector to multiple packet providers. It
// can be popped for packets by the connected packet collector. When this
// happens, the scheduler pops a packet from one of its packet providers based
// on the configured packet filters. The first matching expression determines
// the index of the input gate.
//
simple ContentBasedScheduler extends PacketSchedulerBase like IPacketScheduler
{
    parameters:
        int defaultGateIndex = default(0); // default gate index if no matching labels are found
        string packetFilters; // semicolon separated list of packet filter expressions (e.g. ping-*;udp-*), matches all packets by default
        string packetDataFilters; // semicolon separated list of packet data filter expressions (e.g. inet::Ipv4Header and srcAddress = "10.0.0.1"), matches all packets by default
        @class(ContentBasedScheduler);
}
