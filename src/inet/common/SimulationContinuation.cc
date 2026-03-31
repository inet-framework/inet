//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/SimulationContinuation.h"

namespace inet {

// ---- sleepSimulationTime ----

void sleepSimulationTime(simtime_t duration)
{
    // Schedule the resume event BEFORE suspending, because suspendEvent()
    // does not return until the event is actually resumed.
    resumeEvent(currentEventLoopContext, simTime() + duration);
    suspendEvent();
    // Resumed after wake-up event fired
}

// ---- SimulationContinuation ----

void SimulationContinuation::suspend()
{
    if (isStopped)
        throw cRuntimeError("Cannot suspend event execution because suspend() has been already called");
    isStopped = true;
    // Save the context pointer BEFORE suspending, so resume() can use it later
    suspendedContext = currentEventLoopContext;
    suspendEvent();
    // Resumed after resume() scheduled a wake-up event
}

void SimulationContinuation::resume()
{
    if (!isStopped)
        throw cRuntimeError("Cannot resume event execution because suspend() has not been called yet");
    isStopped = false;
    resumeEvent(suspendedContext, simTime());
    suspendedContext = nullptr;
}

} // namespace inet

