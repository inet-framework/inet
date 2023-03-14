//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.clock;

import inet.applications.contract.IApp;

//
// This module periodically synchronizes the slave clock to the master clock with
// the specified accuracy. The synchronization happens directly using C++ methods
// calls and without any packet exchange. This is primarily useful when the overhead
// of the time synchronization protocol messages can be safely ignored.
//
simple SimpleClockSynchronizer like IApp
{
    parameters:
        string masterClockModule;
        string slaveClockModule;
        volatile double synchronizationInterval @unit(s); // clock synchronization period
        volatile double synchronizationClockTimeError @unit(s) = default(0s); // 0 means slave clock time equals master clock time after synchronization
        volatile double synchronizationOscillatorCompensationError @unit(ppm) = default(0ppm); // 0 means slave clock speed equals master clock speed after synchronization
        @display("i=block/app");
        @lifecycleSupport;
    gates:
        input socketIn @loose;
        output socketOut @loose;
}

