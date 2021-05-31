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

package inet.queueing.meter;

import inet.queueing.base.PacketMeterBase;
import inet.queueing.contract.IPacketMeter;

//
// This module attaches a ~RateTag to each packet specifying the datarate and
// packetrate of the received stream of packets. The algorithm takes packets
// into account only in the given time window.
//
simple SlidingWindowRateMeter extends PacketMeterBase like IPacketMeter
{
    parameters:
        double timeWindow @unit(s); // amount of time to look back from the current simulation time
        @display("i=block/star");
        @class(SlidingWindowRateMeter);
}
