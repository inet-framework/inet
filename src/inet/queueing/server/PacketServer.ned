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

package inet.queueing.server;

import inet.queueing.base.PacketServerBase;
import inet.queueing.contract.IPacketServer;

//
// This module repeatedly pops packets from the connected packet provider and
// after a processing delay it pushes the packet into the connected packet
// consumer. The processing delay is processingTime + packetLength / processingBitrate.
//
simple PacketServer extends PacketServerBase like IPacketServer
{
    parameters:
        volatile double processingTime @unit(s); // determines additional processing time per packet
        volatile double processingBitrate @unit(bps) = default(inf bps); // determines additional processing time per bit
        @class(PacketServer);
}
