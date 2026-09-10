//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIMULATIONCONTINUATION_H
#define __INET_SIMULATIONCONTINUATION_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Stops the execution of the current event and schedules resuming the execution after the specified simulation time.
 */
INET_API void sleepSimulationTime(simtime_t duration);

/**
 * This class supports temporarily stopping and later resuming the execution of the current event.
 */
class INET_API SimulationContinuation
{
  protected:
    bool isStopped = false;
    EventLoopContext *suspendedContext = nullptr;

  public:
    virtual ~SimulationContinuation() { }

    virtual void suspend();
    virtual void resume();
};

} // namespace inet

#endif

