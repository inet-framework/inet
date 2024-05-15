//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/FunctionalEvent.h"

namespace inet {

void schedule(const char *name, simtime_t time, std::function<void()> f)
{
    auto event = new FunctionalEvent(name, f);
    event->setArrivalTime(time);
    getSimulation()->insertEvent(event);
}

void FunctionalEvent::execute()
{
    f();
    // TODO: when f() calls swapcontext it doesn't return, so how should this event be deleted?
    // TODO: it cannot be deleted before calling f() in such a case because that would corrupt the closure's data
    // TODO: perhaps we need a list of objects that should be eventually deleted (in the event loop)
    delete this;
}

} // namespace inet

