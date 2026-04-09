//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.common;

//
// Example module that demonstrates the use of ~SimulationContinuation for
// suspending and resuming event execution.
//
simple SimulationContinuationExample
{
    parameters:
        int count = default(3);
        double maxSleepTime @unit(s) = default(1s);
}
