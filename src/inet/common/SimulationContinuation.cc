//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/SimulationContinuation.h"
#include "inet/common/CoroutineEventExecution.h"

namespace inet {

bool yieldBeforePushPacket = false;

// ---- sleepSimulationTime ----

void sleepSimulationTime(simtime_t duration)
{
    if (currentCoroutineSlot == nullptr)
        throw cRuntimeError("sleepSimulationTime(): coroutine event executor is not installed");
    auto event = new SimulationContextSwitchingEvent("Sleep");
    event->suspendedSlot = currentCoroutineSlot;
    event->setArrivalTime(simTime() + duration);
    getSimulation()->insertEvent(event);
    EV_DEBUG << "Yielding to main loop in sleepSimulationTime()" << EV_ENDL;
    currentCoroutineSlot->yielded = true;
    cCoroutine::switchToMain();
    // Resumed after wake-up event fired
}

// ---- SimulationContinuation ----

void SimulationContinuation::suspend()
{
    if (isStopped)
        throw cRuntimeError("Cannot suspend event execution because suspend() has been already called");
    if (currentCoroutineSlot == nullptr)
        throw cRuntimeError("SimulationContinuation::suspend(): coroutine event executor is not installed");
    isStopped = true;
    suspendedSlot = currentCoroutineSlot;
    EV_DEBUG << "Yielding to main loop in SimulationContinuation::suspend()" << EV_ENDL;
    currentCoroutineSlot->yielded = true;
    cCoroutine::switchToMain();
    // Resumed after resume() scheduled a wake-up event
}

void SimulationContinuation::resume()
{
    if (!isStopped)
        throw cRuntimeError("Cannot resume event execution because suspend() has not been called yet");
    isStopped = false;
    auto event = new SimulationContextSwitchingEvent("Resume");
    event->suspendedSlot = suspendedSlot;
    event->setArrivalTime(simTime());
    getSimulation()->insertEvent(event);
    suspendedSlot = nullptr;
}

// ---- SimulationContextSwitchingEvent ----

void SimulationContextSwitchingEvent::execute()
{
    // no-op — the coroutine executor handles the resume
    // after executeEvent() returns
}

} // namespace inet

