//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/SimulationTaskExample.h"

#include "inet/common/SimulationTask.h"

namespace inet {

Define_Module(SimulationTaskExample);

void SimulationTaskExample::initialize()
{
    scheduleAt(simTime(), new cMessage("start"));
}

void SimulationTaskExample::handleMessage(cMessage *msg)
{
    delete msg;
    runSimulationTaskExample();
}

void SimulationTaskExample::runSimulationTaskExample()
{
    int count = par("count");
    simtime_t maxSleepTime = par("maxSleepTime");
    std::cout << "At: " << simTime() << " parent started" << std::endl;
    SimulationTask parentTask;
    for (int i = 0; i < count; i++) {
        parentTask.spawnChild([=] () {
            std::cout << "At: " << simTime() << " child " << i << " started" << std::endl;
            sleepSimulationTime(uniform(0, maxSleepTime));
            std::cout << "At: " << simTime() << " child " << i << " finished" << std::endl;
        });
    }
    parentTask.joinChildren();
    std::cout << "Parent finished" << std::endl;
}

} // namespace inet
