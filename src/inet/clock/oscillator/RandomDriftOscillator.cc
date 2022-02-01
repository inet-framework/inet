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
        driftRateParameter = &par("driftRate");
        driftRateChangeParameter = &par("driftRateChange");
        changeIntervalParameter = &par("changeInterval");
        driftRate = driftRateParameter->doubleValue() / 1E+6;
    }
    DriftingOscillatorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        changeTimer = new cMessage("ChangeTimer");
        driftRateChangeLowerLimit = par("driftRateChangeLowerLimit").doubleValue() / 1E+6;
        driftRateChangeUpperLimit = par("driftRateChangeUpperLimit").doubleValue() / 1E+6;
        scheduleAfter(changeIntervalParameter->doubleValue(), changeTimer);
    }
}

void RandomDriftOscillator::handleMessage(cMessage *message)
{
    if (message == changeTimer) {
        driftRateChangeTotal += driftRateChangeParameter->doubleValue() / 1E+6;
        driftRateChangeTotal = std::max(driftRateChangeTotal, driftRateChangeLowerLimit);
        driftRateChangeTotal = std::min(driftRateChangeTotal, driftRateChangeUpperLimit);
        auto driftRate = driftRateParameter->doubleValue() / 1E+6;
        setDriftRate(driftRate + driftRateChangeTotal);
        scheduleAfter(changeIntervalParameter->doubleValue(), changeTimer);
    }
    else
        throw cRuntimeError("Unknown message");
}

} // namespace inet

