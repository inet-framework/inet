//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/configurator/gatescheduling/common/AlwaysOpenGateScheduleConfigurator.h"

namespace inet {

Define_Module(AlwaysOpenGateScheduleConfigurator);

AlwaysOpenGateScheduleConfigurator::Output *AlwaysOpenGateScheduleConfigurator::computeGateScheduling(const Input& input) const
{
    auto output = new Output();
    for (auto port : input.ports) {
        for (int gateIndex = 0; gateIndex < port->numGates; gateIndex++) {
            Output::Schedule *schedule = new Output::Schedule();
            schedule->port = port;
            schedule->gateIndex = gateIndex;
            schedule->cycleStart = 0;
            schedule->cycleDuration = gateCycleDuration;
            schedule->open = false;
            output->gateSchedules[port].push_back(schedule);
        }
    }
    for (auto application : input.applications)
        output->applicationStartTimes[application] = 0;
    return output;
}

} // namespace inet

