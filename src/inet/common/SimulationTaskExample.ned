//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.common;

//
// Example module that demonstrates the use of ~SimulationTask for spawning
// child simulation tasks that run concurrently in simulation time.
//
simple SimulationTaskExample
{
    parameters:
        int count = default(3);
        double maxSleepTime @unit(s) = default(1s);
}
