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
// This is a base module for various packet sink modules.
//
// @see ~IPacketSink
//
simple PacketSinkBase extends PacketProcessorBase
{
    parameters:
        displayStringTextFormat = default("dropped %p pk (%l)");
        @class(PacketSinkBase);
        @display("i=block/sink");
        @signal[packetDropped](type=inet::Packet);
        // the statistical value is the dropped packet
        @statistic[droppedPackets](title="dropped packets"; source=packetDropped; record=count; unit=pk);
        // the statistical value is the length of the dropped packet
        @statistic[droppedPacketLengths](title="dropped packet lengths"; source=packetLength(packetDropped); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the dropped packets
        @statistic[droppedDataRate](title="dropped data rate"; source=throughput(packetDropped); record=vector; unit=bps; interpolationmode=linear);
}
