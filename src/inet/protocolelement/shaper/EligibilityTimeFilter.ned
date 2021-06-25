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

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// This module is a packet filter that operates based on the ~EligibilityTimeTag
// attached to the packet that is passing through. Packets are dropped if the tag
// is missing. If the maxResidenceTime parameter is set, then the filter also drops
// the packets for which the eligibility time in the attached ~EligibilityTimeTag
// is greater than the current simulation time plus the maximum residence time.
//
// This module is part of the asynchronous shaper architecture.
//
// @see ~EligibilityTimeGate, ~EligibilityTimeQueue, ~EligibilityTimeMeter, ~EligibilityTimeTag
//
simple EligibilityTimeFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        double maxResidenceTime @unit(s) = default(-1s); // optional max residence time limit, not used by default
        @class(EligibilityTimeFilter);
}
