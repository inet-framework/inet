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
    cPar *synchronizationAccuracyParameter = nullptr;

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

