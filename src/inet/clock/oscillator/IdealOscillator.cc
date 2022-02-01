//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
        if (tickLength == 0)
            tickLength.setRaw(1);
        WATCH(tickLength);
        emit(driftRateChangedSignal, 0.0);
    }
}

void IdealOscillator::finish()
{
    OscillatorBase::finish();
    emit(driftRateChangedSignal, 0.0);
}

int64_t IdealOscillator::computeTicksForInterval(simtime_t timeInterval) const
{
    return timeInterval.raw() / tickLength.raw();
}

simtime_t IdealOscillator::computeIntervalForTicks(int64_t numTicks) const
{
    return tickLength * numTicks;
}

} // namespace inet

