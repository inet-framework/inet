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

package inet.common.queueing;

import inet.common.queueing.base.PacketSinkBase;
import inet.common.queueing.contract.IPacketConsumer;

//
// This module is a passive packet sink which is pushed with packets by the
// connected packet producer. All pushed packets are counted and deleted.
//
simple PacketConsumer extends PacketSinkBase like IPacketConsumer
{
    parameters:
        volatile double consumptionInterval @unit(s) = default(0s); // elapsed time between two subsequent packets pushed by the connected packet producer
        @class(PacketConsumer);
        @signal[packetPushed](type=inet::Packet);
    gates:
        input in @labels(push);
}

