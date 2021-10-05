//
// Copyright (C) 2020 OpenSim Ltd.
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
// This is a base module for various packet filter modules. Derived modules must
// implement a single packet matcher function which determines if a packet is to
// be passed through or filtered out.
//
// @see ~IPacketFilter
//
simple PacketFilterBase extends PacketProcessorBase
{
    parameters:
        bool backpressure = default(false);
        displayStringTextFormat = default("dropped %d/%p pk (%k/%l)"); // determines the text that is written on top of the submodule
        @class(PacketFilterBase);
        @display("i=block/downarrow");
        @signal[packetPushed](type=inet::Packet);
        @signal[packetPulled](type=inet::Packet);
        @signal[packetFiltered](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetPushed](title="packets pushed"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetPulled](title="packets pulled"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetFiltered](title="packets filtered"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropped](title="packets dropped"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[pushedTraffic](title="pushed traffic"; source=throughput(packetPushed); record=vector; unit=bps; interpolationmode=linear);
        @statistic[filteredTraffic](title="filtered traffic"; source=throughput(packetFiltered); record=vector; unit=bps; interpolationmode=linear);
        @statistic[droppedTraffic](title="dropped traffic"; source=throughput(packetDropped); record=vector; unit=bps; interpolationmode=linear);
        @statistic[flowPushedTraffic](title="flow pushed traffic"; source=throughput(flowPacketLength(demuxFlow(packetPushed))); record=vector; unit=bps; interpolationmode=linear);
        @statistic[flowFilteredTraffic](title="flow filtered traffic"; source=throughput(flowPacketLength(demuxFlow(packetFiltered))); record=vector; unit=bps; interpolationmode=linear);
        @statistic[flowDroppedTraffic](title="flow dropped traffic"; source=throughput(flowPacketLength(demuxFlow(packetDropped))); record=vector; unit=bps; interpolationmode=linear);
    gates:
        input in @labels(send,push,pull,pass,stream);
        output out @labels(send,push,pull,pass,stream);
}
