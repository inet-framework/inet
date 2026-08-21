//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/FunctionalEventExample.h"

#include "inet/common/FunctionalEvent.h"

namespace inet {

Define_Module(FunctionalEventExample);

void FunctionalEventExample::initialize()
{
    int count = par("count");
    simtime_t maxDelay = par("maxDelay");
    for (int i = 0; i < count; i++) {
        simtime_t delay = uniform(0, maxDelay);
        inet::scheduleAfter("scheduleAfter", delay, [=] () {
            std::cout << "At: " << simTime() << " scheduleAfter event " << i << " executed" << std::endl;
        });
        std::cout << "At: " << simTime() << " scheduled scheduleAfter event " << i << " with delay " << delay << std::endl;
        simtime_t time = simTime() + uniform(0, maxDelay);
        inet::scheduleAt("scheduleAt", time, [=] () {
            std::cout << "At: " << simTime() << " scheduleAt event " << i << " executed" << std::endl;
        });
        std::cout << "At: " << simTime() << " scheduled scheduleAt event " << i << " at time " << time << std::endl;
    }
}

} // namespace inet
