//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIMULATIONCONTINUATION_H
#define __INET_SIMULATIONCONTINUATION_H

#include "inet/common/INETDefs.h"

namespace inet {

struct CoroutineSlot;

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
    CoroutineSlot *suspendedSlot = nullptr;

  public:
    virtual ~SimulationContinuation() { }

    virtual void suspend();
    virtual void resume();
};

class INET_API SimulationContextSwitchingEvent : public cEvent
{
  public:
    CoroutineSlot *suspendedSlot = nullptr;

  public:
    SimulationContextSwitchingEvent(const char *name) : cEvent(name) { }

    virtual cEvent *dup() const override { throw cRuntimeError("Cannot duplicate SimulationContextSwitchingEvent"); }
    virtual cObject *getTargetObject() const override { return nullptr; }

    virtual void execute() override;
};

/**
 * When enabled, inserts a yield (sleepSimulationTime(0)) before each new pushPacket()
 * call that replaced a send() during the intra-node communication refactoring.
 * This recreates the old event-by-event processing for trajectory verification.
 */
INET_API extern bool yieldBeforePushPacket;

inline void yieldBeforePush() {
    if (yieldBeforePushPacket)
        sleepSimulationTime(0);
}

} // namespace inet

#endif

