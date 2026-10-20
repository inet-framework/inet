//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/FunctionalEvent.h"

namespace inet {

void scheduleAt(const char *name, simtime_t time, std::function<void()> f)
{
    if (time < simTime())
        throw cRuntimeError("Cannot schedule function to the past");
    auto event = new FunctionalEvent(name, f);
    event->setArrivalTime(time);
    getSimulation()->insertEvent(event);
}

void scheduleAfter(const char *name, simtime_t delay, std::function<void()> f)
{
    if (delay < 0)
        throw cRuntimeError("Cannot schedule function to the past");
    scheduleAt(name, simTime() + delay, f);
}

void FunctionalEvent::execute()
{
    f();
    removeFromOwnershipTree();
    delete this;
}

} // namespace inet

