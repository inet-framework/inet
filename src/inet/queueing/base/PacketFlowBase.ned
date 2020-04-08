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
// This is a base module for various packet flow modules. A packet flow module
// passes or streams all pushed or pulled packets after processing them from
// its input to its output.
//
// @see ~IPacketFlow
//
simple PacketFlowBase extends PacketProcessorBase
{
    parameters:
        @class(PacketFlowBase);
        @display("i=block/rightarrow");
        @signal[packetPushed](type=inet::Packet);
        @signal[packetPulled](type=inet::Packet);
        @statistic[packetPushed](title="packets pushed"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetPulled](title="packets pulled"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    gates:
        input in @labels(send,push,pull,pass,stream);
        output out @labels(send,push,pull,pass,stream);
}
