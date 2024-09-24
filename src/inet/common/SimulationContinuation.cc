//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/SimulationContinuation.h"

namespace inet {

static void runSimulationEventLoop()
{
    EV_DEBUG << "Running event loop" << EV_ENDL;
    cSimulation::getActiveSimulation()->setGlobalContext();
    cSimulation::getActiveEnvir()->runEventLoop();
}

void sleepSimulationTime(simtime_t duration)
{
    ucontext_t newContext;
    int stackSize = 16384;
    newContext.uc_stack.ss_sp = new char[stackSize];
    newContext.uc_stack.ss_size = stackSize;
    if (getcontext(&newContext) != 0)
        throw cRuntimeError("Cannot get current context");
    makecontext(&newContext, (void (*)(void))runSimulationEventLoop, 0);
    auto event = new SimulationContextSwitchingEvent("Sleep");
    event->oldContextType = cSimulation::getActiveSimulation()->getContextType();
    event->setArrivalTime(simTime() + duration);
    getSimulation()->insertEvent(event);
    EV_DEBUG << "Calling swapcontext() in sleepSimulationTime()" << EV_ENDL;
    swapcontext(&event->oldContext, &newContext);
}

void SimulationContinuation::suspend()
{
    if (isStopped)
        throw cRuntimeError("Cannot suspend event execution because suspend() has been already called");
    ucontext_t newContext;
    int stackSize = 16384;
    newContext.uc_stack.ss_sp = new char[stackSize];
    newContext.uc_stack.ss_size = stackSize;
    if (getcontext(&newContext) != 0)
        throw cRuntimeError("Cannot get current context");
    makecontext(&newContext, (void (*)(void)) (runSimulationEventLoop), 0);
    isStopped = true;
    oldContextType = cSimulation::getActiveSimulation()->getContextType();
    EV_DEBUG << "Calling swapcontext() in SimulationContinuation::suspend()" << EV_ENDL;
    swapcontext(&oldContext, &newContext);
}

void SimulationContinuation::resume()
{
    if (!isStopped)
        throw cRuntimeError("Cannot resume event execution because suspend() has not been called yet");
    ucontext_t newContext;
    cSimulation::getActiveSimulation()->setContextType(oldContextType);
    EV_DEBUG << "Calling swapcontext() in SimulationContinuation::resume()" << EV_ENDL;
    swapcontext(&newContext, &oldContext);
}

void SimulationContextSwitchingEvent::execute()
{
    ucontext_t newContext;
    cSimulation::getActiveSimulation()->setContextType(oldContextType);
    EV_DEBUG << "Calling swapcontext() in SimulationContextSwitchingEvent::execute()" << EV_ENDL;
    swapcontext(&newContext, &oldContext);
    // TODO: swapcontext doesn't return, so how do we delete this event?
    // TODO: we can't delete this event before calling swapcontext because that would corrupt its data
}

} // namespace inet

