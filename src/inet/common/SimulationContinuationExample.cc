//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/SimulationContinuationExample.h"

#include "inet/common/FunctionalEvent.h"
#include "inet/common/SimulationContinuation.h"

namespace inet {

Define_Module(SimulationContinuationExample);

void SimulationContinuationExample::initialize()
{
    scheduleAt(simTime(), new cMessage("start"));
}

void SimulationContinuationExample::handleMessage(cMessage *msg)
{
    delete msg;
    runSimulationContinuationExample();
}

void SimulationContinuationExample::runSimulationContinuationExample()
{
    int count = par("count");
    simtime_t maxSleepTime = par("maxSleepTime");
    for (int i = 0; i < count; i++) {
        SimulationContinuation continuation;
        inet::scheduleAfter("resume", uniform(0, maxSleepTime), [&] () {
            std::cout << "At: " << simTime() << " step " << i << " resuming" << std::endl;
            continuation.resume();
        });
        std::cout << "At: " << simTime() << " step " << i << " suspending" << std::endl;
        continuation.suspend();
        std::cout << "At: " << simTime() << " step " << i << " resumed" << std::endl;
    }
    std::cout << "At: " << simTime() << " all steps finished" << std::endl;
}

} // namespace inet
