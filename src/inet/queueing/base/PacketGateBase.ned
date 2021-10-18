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
// This is a base module for various packet gate modules.
//
// @see ~IPacketGate
//
simple PacketGateBase extends PacketFlowBase
{
    parameters:
        displayStringTextFormat = default("passed %p pk (%l)");
        double bitrate @unit(bps) = default(nan bps);
        int extraLength @unit(b) = default(0b);
        double extraDuration @unit(s) = default(0s);
        @class(PacketGateBase);
        @display("i=block/cogwheel");
        @signal[gateStateChanged](type=bool);
        @statistic[gateState](title="gate state"; type=enum; enum=CLOSED, OPEN; source=gateStateChanged; record=vector; interpolationmode=sample-hold);
}
