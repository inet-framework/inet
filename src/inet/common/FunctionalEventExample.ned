//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.common;

//
// Example module that demonstrates the use of ~FunctionalEvent for scheduling
// lambda functions at specific simulation times.
//
simple FunctionalEventExample
{
    parameters:
        int count = default(3);
        double maxDelay @unit(s) = default(1s);
}
