//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/configurator/gatescheduling/AlwaysOpenGateSchedulingConfigurator.h"

namespace inet {

Define_Module(AlwaysOpenGateSchedulingConfigurator);

AlwaysOpenGateSchedulingConfigurator::Output *AlwaysOpenGateSchedulingConfigurator::computeGateScheduling(const Input& input) const
{
    auto output = new Output();
    for (auto port : input.ports) {
        for (int priority = 0; priority < port->numPriorities; priority++) {
            Output::Schedule *schedule = new Output::Schedule();
            schedule->port = port;
            schedule->gateIndex = priority;
            schedule->cycleStart = 0;
            schedule->cycleDuration = gateCycleDuration;
            Output::Slot slot;
            slot.start = 0;
            slot.duration = gateCycleDuration;
            schedule->slots.push_back(slot);
            output->gateSchedules[port].push_back(schedule);
        }
    }
    for (auto application : input.applications)
        output->applicationStartTimes[application] = 0;
    return output;
}

} // namespace inet

