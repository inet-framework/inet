//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.clock;

import inet.applications.contract.IApp;

//
// Implements a lightweight clock synchronization mechanism that periodically aligns
// a slave clock with a master clock at configurable intervals and with adjustable
// precision. Performs direct synchronization through C++ method calls without
// simulating network packet exchanges, providing an efficient way to model time
// synchronization in scenarios where protocol overhead is negligible.
//
// Supports configurable synchronization accuracy through parameters that control
// both the time error (difference between master and slave clock times after
// synchronization) and oscillator compensation error (difference in clock speeds
// after synchronization). These error parameters enable realistic modeling of
// imperfect synchronization scenarios while maintaining simulation efficiency.
// Particularly useful for simulating distributed systems that require synchronized
// clocks without the overhead of explicit synchronization protocols.
//
// @see ~IClock, ~OscillatorBasedClock
//
simple SimpleClockSynchronizer like IApp
{
    parameters:
        string masterClockModule;
        string slaveClockModule;
        volatile double synchronizationInterval @unit(s); // Clock synchronization period
        volatile double synchronizationClockTimeError @unit(s) = default(0s); // 0 means slave clock time equals master clock time after synchronization
        volatile double synchronizationOscillatorCompensationError @unit(ppm) = default(0ppm); // 0 means slave clock speed equals master clock speed after synchronization
        @display("i=block/app");
        @lifecycleSupport;
    gates:
        input socketIn @loose;
        output socketOut @loose;
}
