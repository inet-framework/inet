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

package inet.queueing.source;

import inet.queueing.base.PassivePacketSourceBase;
import inet.queueing.contract.IPassivePacketSource;

//
// This module is a passive packet source which can be pulled for packets from
// the connected packet collector.
//
simple PassivePacketSource extends PassivePacketSourceBase like IPassivePacketSource
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        volatile double providingInterval @unit(s) = default(0s); // elapsed time between subsequent packets allowed to be pulled by the connected packet collector, 0 means any number of packets can be pulled at the same simulation time
        bool scheduleForAbsoluteTime = default(true); // when a clock is used relative means that setting the clock will not affect the simulation time of the event
        @class(PassivePacketSource);
    gates:
        output out @labels(pull);
}
