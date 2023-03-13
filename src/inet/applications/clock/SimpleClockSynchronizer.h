//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIMPLECLOCKSYNCHRONIZER_H
#define __INET_SIMPLECLOCKSYNCHRONIZER_H

#include <functional>

#include "inet/applications/base/ApplicationBase.h"
#include "inet/clock/model/SettableClock.h"
#include "inet/common/ModuleRefByPar.h"

namespace inet {

class INET_API SimpleClockSynchronizer : public ApplicationBase
{
  protected:
    cMessage *synhronizationTimer = nullptr;
    ModuleRefByPar<IClock> masterClock;
    ModuleRefByPar<SettableClock> slaveClock;
    cPar *synchronizationIntervalParameter = nullptr;
    cPar *synchronizationClockTimeErrorParameter = nullptr;
    cPar *synchronizationOscillatorCompensationErrorParameter = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override { }
    virtual void handleCrashOperation(LifecycleOperation *operation) override { }

    virtual void synchronizeSlaveClock();
    virtual void scheduleSynchronizationTimer();

  public:
    virtual ~SimpleClockSynchronizer() { cancelAndDelete(synhronizationTimer); }
};

} // namespace

#endif

