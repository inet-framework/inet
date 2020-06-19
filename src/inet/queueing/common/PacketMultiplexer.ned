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

package inet.queueing.common;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IActivePacketSource;

//
// This module connects multiple packet producers to one packet consumer. When
// a packet is pushed by one of the connected packet producers, then the multiplexer
// simply pushes the packet to the connected packet consumer.
//
simple PacketMultiplexer extends PacketProcessorBase like IActivePacketSource // TODO: IPassivePacketSink // in gate is vector
{
    parameters:
        displayStringTextFormat = default("passed %p pk (%l)"); // %p number of packet, %l total length
        @class(PacketMultiplexer);
        @display("i=-;b=10,10,oval,grey,,1");
        @signal[packetPushed](type=inet::Packet);
        @signal[packetPulled](type=inet::Packet);
        @statistic[packetPushed](title="packets pushed"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetPulled](title="packets pulled"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    gates:
        input in[] @labels(push);
        output out @labels(push);
}
