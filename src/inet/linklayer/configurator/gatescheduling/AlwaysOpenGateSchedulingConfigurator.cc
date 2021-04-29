//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
            schedule->priority = priority;
            schedule->cycleStart = 0;
            schedule->cycleDuration = gateCycleDuration;
            schedule->slotStarts.push_back(0);
            schedule->slotDurations.push_back(gateCycleDuration);
            output->gateSchedules[port].push_back(schedule);
        }
    }
    for (auto application : input.applications)
        output->applicationStartTimes[application] = 0;
    return output;
}

} // namespace inet

