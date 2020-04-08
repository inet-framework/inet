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
// This is a base module for various packet server modules.
//
// @see ~IPacketServer
//
simple PacketServerBase extends PacketSinkBase
{
    parameters:
        displayStringTextFormat = default("served %p pk (%l)");
        @display("i=block/server");
        @signal[packetServed](type=inet::Packet);
        @statistic[packetServed](title="packets served"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    gates:
        input in;
        output out;
}
