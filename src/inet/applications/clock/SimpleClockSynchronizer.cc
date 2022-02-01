//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/clock/SimpleClockSynchronizer.h"

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
        synchronizationAccuracyParameter = &par("synchronizationAccuracy");
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

void SimpleClockSynchronizer::synchronizeSlaveClock()
{
    slaveClock->setClockTime(masterClock->getClockTime() + synchronizationAccuracyParameter->doubleValue(), true);
}

void SimpleClockSynchronizer::scheduleSynchronizationTimer()
{
    scheduleAfter(synchronizationIntervalParameter->doubleValue(), synhronizationTimer);
}

} // namespace

