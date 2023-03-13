//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/oscillator/RandomDriftOscillator.h"

namespace inet {

Define_Module(RandomDriftOscillator);

void RandomDriftOscillator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        driftRateChangeParameter = &par("driftRateChange");
        changeIntervalParameter = &par("changeInterval");
        driftRate = initialDriftRate = ppm(par("initialDriftRate"));
    }
    DriftingOscillatorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        changeTimer = new cMessage("ChangeTimer");
        driftRateChangeLowerLimit = ppm(par("driftRateChangeLowerLimit"));
        driftRateChangeUpperLimit = ppm(par("driftRateChangeUpperLimit"));
        scheduleAfter(changeIntervalParameter->doubleValue(), changeTimer);
    }
}

void RandomDriftOscillator::handleMessage(cMessage *message)
{
    if (message == changeTimer) {
        driftRateChangeTotal += ppm(driftRateChangeParameter->doubleValue());
        driftRateChangeTotal = std::max(driftRateChangeTotal, driftRateChangeLowerLimit);
        driftRateChangeTotal = std::min(driftRateChangeTotal, driftRateChangeUpperLimit);
        setDriftRate(initialDriftRate + driftRateChangeTotal);
        scheduleAfter(changeIntervalParameter->doubleValue(), changeTimer);
    }
    else
        throw cRuntimeError("Unknown message");
}

} // namespace inet

