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
        volatile double synchronizationAccuracy @unit(s) = default(0s); // 0 means exact synchronization
        @display("i=block/app");
        @lifecycleSupport;
    gates:
        input socketIn @loose;
        output socketOut @loose;
}

