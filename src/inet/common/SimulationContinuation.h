//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIMULATIONCONTINUATION_H
#define __INET_SIMULATIONCONTINUATION_H

#include "inet/common/INETDefs.h"

// NOTE: use the following library if ucontext_t is not supported on your platform: https://github.com/kaniini/libucontext

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
    ucontext_t oldContext;
    ContextType oldContextType;

  public:
    virtual ~SimulationContinuation() { }

    virtual void suspend();
    virtual void resume();
};

class INET_API SimulationContextSwitchingEvent : public cEvent
{
  public:
    ucontext_t oldContext;
    ContextType oldContextType;

  public:
    SimulationContextSwitchingEvent(const char *name) : cEvent(name) { }

    virtual cEvent *dup() const override { throw cRuntimeError("Cannot duplicate SimulationContextSwitchingEvent"); }
    virtual cObject *getTargetObject() const override { return nullptr; }

    virtual void execute() override;
};

} // namespace inet

#endif

