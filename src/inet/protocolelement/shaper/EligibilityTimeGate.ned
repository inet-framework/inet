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

package inet.protocolelement.shaper;

import inet.queueing.base.PacketGateBase;
import inet.queueing.contract.IPacketGate;

//
// This module is a packet gate that operates based on the ~EligibilityTimeTag
// attached to the next packet waiting to be pulled through. The gate is closed
// if the eligibility time is greater than the current simulation time, it is
// open otherwise.
//
// This module is part of the asynchronous shaper architecture.
//
// @see ~EligibilityTimeQueue, ~EligibilityTimeFilter, ~EligibilityTimeMeter, ~EligibilityTimeTag
//
simple EligibilityTimeGate extends PacketGateBase like IPacketGate
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        @class(EligibilityTimeGate);
}
