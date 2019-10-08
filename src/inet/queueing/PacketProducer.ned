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

package inet.queueing;

import inet.queueing.base.PacketSourceBase;
import inet.queueing.contract.IPacketProducer;

//
// This module is an active packet source which pushes packets into the connected
// packet consumer.
//
simple PacketProducer extends PacketSourceBase like IPacketProducer
{
    parameters:
        volatile double productionInterval @unit(s); // elapsed time between subsequent packets pushed to the connected packet consumer
        @class(PacketProducer);
        @display("i=block/source");
        @signal[packetCreated](type=inet::Packet);
        @statistic[packetCreated](title="created packets"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    gates:
        output out @labels(push);
}
