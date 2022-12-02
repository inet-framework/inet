//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/clock/SimpleClockSynchronizer.h"

#include "inet/clock/base/DriftingOscillatorBase.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

Define_Module(SimpleClockSynchronizer);

void SimpleClockSynchronizer::initialize(int stage)
{
    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        synhronizationTimer = new cMessage("SynchronizationTimer");
        masterClock.reference(this, "masterClockModule", true);
        slaveClock.reference(this, "slaveClockModule", true);
        synchronizationIntervalParameter = &par("synchronizationInterval");
        synchronizationClockTimeErrorParameter = &par("synchronizationClockTimeError");
        synchronizationOscillatorCompensationFactorErrorParameter = &par("synchronizationOscillatorCompensationFactorError");
    }
}

void SimpleClockSynchronizer::handleMessageWhenUp(cMessage *msg)
{
    if (msg == synhronizationTimer) {
        synchronizeSlaveClock();
        scheduleSynchronizationTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void SimpleClockSynchronizer::handleStartOperation(LifecycleOperation *operation)
{
    scheduleSynchronizationTimer();
}

static double getCurrentTickLength(IClock *clock)
{
    auto oscillatorBasedClock = check_and_cast<OscillatorBasedClock*>(clock);
    auto clockOscillator = oscillatorBasedClock->getOscillator();
    auto driftingOscillator = check_and_cast<const DriftingOscillatorBase *>(clockOscillator);
    return driftingOscillator->getCurrentTickLength();
}

void SimpleClockSynchronizer::synchronizeSlaveClock()
{
    auto masterOscillatorBasedClock = check_and_cast<OscillatorBasedClock*>(masterClock.get());
    auto clockTime = masterClock->getClockTime() + synchronizationClockTimeErrorParameter->doubleValue();
    double oscillatorCompensationFactor = getCurrentTickLength(slaveClock.get()) / getCurrentTickLength(masterClock.get()) * masterOscillatorBasedClock->getOscillatorCompensationFactor() * synchronizationOscillatorCompensationFactorErrorParameter->doubleValue();
    slaveClock->setClockTime(clockTime, oscillatorCompensationFactor, true);
}

void SimpleClockSynchronizer::scheduleSynchronizationTimer()
{
    scheduleAfter(synchronizationIntervalParameter->doubleValue(), synhronizationTimer);
}

} // namespace

