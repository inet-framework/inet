//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
// This is a base module for various packet scheduler modules. Derived modules
// must implement a single packet scheduler function which determines the index
// of the input gate for the pulled packet.
//
// @see ~IPacketScheduler
//
simple PacketSchedulerBase extends PacketProcessorBase
{
    parameters:
        bool reverseOrder = default(false);
        displayStringTextFormat = default("scheduled %p pk (%l)");
        @class(PacketSchedulerBase);
        @display("i=block/join");
        @signal[packetPulled](type=inet::Packet);
        // the statistical value is the scheduled packet
        @statistic[packets](title="packets"; source=packetPulled; record=count; unit=pk);
        // the statistical value is the length of the scheduled packet
        @statistic[packetLengths](title="packet lengths"; source=packetLength(packetPulled); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the scheduled packets
        @statistic[dataRate](title="data rate"; source=throughput(packetPulled); record=vector; unit=bps; interpolationmode=linear);
    gates:
        input in[];
        output out;
}
