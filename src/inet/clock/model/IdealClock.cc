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

#include "inet/clock/model/IdealClock.h"

namespace inet {

Define_Module(IdealClock);

void IdealClock::initialize(int stage)
{
    ClockBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        emit(timeChangedSignal, simTime());
}

void IdealClock::finish()
{
    ClockBase::finish();
    emit(timeChangedSignal, simTime());
}

clocktime_t IdealClock::computeClockTimeFromSimTime(simtime_t t) const
{
    return ClockTime::from(t);
}

simtime_t IdealClock::computeSimTimeFromClockTime(clocktime_t clock) const
{
    return clock.asSimTime();
}

} // namespace inet

