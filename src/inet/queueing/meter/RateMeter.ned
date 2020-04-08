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

package inet.queueing.meter;

import inet.queueing.base.PacketMeterBase;
import inet.queueing.contract.IPacketMeter;

//
// This module attaches a ~RateTag to each packet specifying the datarate and
// packetrate of the received stream of packets. The algorithm takes older
// packets into account with an exponentially decreasing weight.
//
simple RateMeter extends PacketMeterBase like IPacketMeter
{
    parameters:
        double alpha; // parameter in the range of (0, 1) for the exponential weight decrease
        @display("i=block/star");
        @class(RateMeter);
}
