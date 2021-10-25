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

package inet.queueing.shaper;

//
// This module implements an asynchronous shaper. The queue sorts packets based
// on the transmission eligibility time (~EligibilityTimeTag) attached to each
// packet. The gate prevents packets to be transmitter earlier than the attached
// transmission eligibility time.
//
// @see ~EligibilityTimeMeter, ~EligibilityTimeFilter, ~EligibilityTimeQueue, ~EligibilityTimeGate
//
module AsynchronousShaper extends PacketShaper
{
    parameters:
        queue.typename = default("EligibilityTimeQueue");
        gate.typename = default("EligibilityTimeGate");
}
