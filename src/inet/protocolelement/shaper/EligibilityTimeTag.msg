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

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.clock.common.ClockTime;

namespace inet;

//
// This tag specifies the simulation time after which the packet becomes eligible
// for transmission according to the asynchronous shaper.
//
// This tag is used by the asynchronous shaper infrastructure.
//
// @see ~EligibilityTimeGate, ~EligibilityTimeFilter, ~EligibilityTimeMeter, ~EligibilityTimeQueue
//
class EligibilityTimeTag extends TagBase
{
    clocktime_t eligibilityTime; // the simulation time after which the packet is eligible for transmission
}
