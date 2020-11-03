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

#include "inet/clock/oscillator/IdealOscillator.h"

namespace inet {

Define_Module(IdealOscillator);

void IdealOscillator::initialize(int stage)
{
    OscillatorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        origin = simTime();
        tickLength = par("tickLength");
        WATCH(tickLength);
        emit(driftRateChangedSignal, 0);
    }
}

void IdealOscillator::finish()
{
    OscillatorBase::finish();
    emit(driftRateChangedSignal, 0);
}

int64_t IdealOscillator::computeTicksForInterval(simtime_t timeInterval) const
{
    return (int64_t)floor(timeInterval / tickLength);
}

simtime_t IdealOscillator::computeIntervalForTicks(int64_t numTicks) const
{
    return tickLength * numTicks;
}

} // namespace inet

