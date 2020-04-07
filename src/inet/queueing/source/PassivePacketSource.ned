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

package inet.queueing.source;

import inet.queueing.base.PacketSourceBase;
import inet.queueing.contract.IPassivePacketSource;

//
// This module is a passive packet source which can be pulled for packets from
// the connected packet collector.
//
simple PassivePacketSource extends PacketSourceBase like IPassivePacketSource
{
    parameters:
        volatile double providingInterval @unit(s) = default(0s); // elapsed time between subsequent packets allowed to be pulled by the connected packet collector, 0 means any number of packets can be pulled at the same simulation time
        @class(PassivePacketSource);
        @signal[packetPulled](type=inet::Packet);
        @signal[packetCreated](type=inet::Packet);
        @statistic[packetPulled](title="packets pulled"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetCreated](title="packets created"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    gates:
        output out @labels(pull);
}
