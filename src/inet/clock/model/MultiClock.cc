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

#include "inet/clock/model/MultiClock.h"

#include "inet/clock/base/ClockBase.h"

namespace inet {

Define_Module(MultiClock);

void MultiClock::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        activeClock = check_and_cast<IClock *>(getSubmodule("clock", par("activeClockIndex")));
        subscribe(ClockBase::timeChangedSignal, this);
    }
}

void MultiClock::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "activeClockIndex")) {
            emit(ClockBase::timeChangedSignal, getClockTime().asSimTime());
            activeClock = check_and_cast<IClock *>(getSubmodule("clock", par("activeClockIndex")));
            emit(ClockBase::timeChangedSignal, getClockTime().asSimTime());
        }
    }
}

void MultiClock::receiveSignal(cComponent *source, int signal, const simtime_t& time, cObject *details)
{
    if (signal == ClockBase::timeChangedSignal) {
        if (check_and_cast<IClock *>(source) == activeClock)
            emit(ClockBase::timeChangedSignal, time, details);
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace inet

