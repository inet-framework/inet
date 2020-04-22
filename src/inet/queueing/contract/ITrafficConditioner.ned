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

package inet.queueing.contract;

//
// This module interface is implemented by traffic conditioner modules. A traffic
// conditioner connects one input to one output. It operates in passive mode on
// its input and in active mode on its output. Packets can be pushed into its
// input and packets can be pulled from its output. A traffic conditioner can
// drop, delay and reorder packets, but it doesn't modify packets in any way.
//
moduleinterface ITrafficConditioner extends IPassivePacketSink, IActivePacketSource
{
    parameters:
        @display("i=block/downarrow");
}
