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
        synchronizationOscillatorCompensationErrorParameter = &par("synchronizationOscillatorCompensationError");
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

static double getCurrentRelativeTickLength(IClock *clock)
{
    auto oscillatorBasedClock = check_and_cast<OscillatorBasedClock*>(clock);
    auto clockOscillator = oscillatorBasedClock->getOscillator();
    auto driftingOscillator = check_and_cast<const DriftingOscillatorBase *>(clockOscillator);
    return driftingOscillator->getCurrentTickLength() / driftingOscillator->getNominalTickLength();
}

void SimpleClockSynchronizer::synchronizeSlaveClock()
{
    auto masterOscillatorBasedClock = check_and_cast<OscillatorBasedClock*>(masterClock.get());
    auto clockTime = masterClock->getClockTime() + synchronizationClockTimeErrorParameter->doubleValue();
    ppm oscillatorCompensation = unit(getCurrentRelativeTickLength(slaveClock.get()) / getCurrentRelativeTickLength(masterClock.get())
            * (1 + unit(masterOscillatorBasedClock->getOscillatorCompensation()).get())
            * (1 + unit(ppm(synchronizationOscillatorCompensationErrorParameter->doubleValue())).get()) - 1);
    slaveClock->setClockTime(clockTime, oscillatorCompensation, true);
}

void SimpleClockSynchronizer::scheduleSynchronizationTimer()
{
    scheduleAfter(synchronizationIntervalParameter->doubleValue(), synhronizationTimer);
}

} // namespace

