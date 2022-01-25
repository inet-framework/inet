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

package inet.queueing.gate;

import inet.queueing.base.PacketGateBase;
import inet.queueing.contract.IPacketGate;

//
// This module allows or forbids packets to pass through depending on whether
// the gate is open or closed. The gate is open and closed according to the list
// of change times periodically.
//
simple PeriodicGate extends PacketGateBase like IPacketGate
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        bool initiallyOpen @mutable = default(true); // specifies if the gate is initially open or closed
        double offset @mutable @unit(s) = default(0s); // specifies where the period starts in the list of gate state change durations
        object durations @mutable @unit(s) = default([]); // list of time durations within the period that determine if the gate is open or closed
        bool scheduleForAbsoluteTime = default(true); // when a clock is used relative means that setting the clock will not affect the simulation time of the event
        @class(PeriodicGate);
}
