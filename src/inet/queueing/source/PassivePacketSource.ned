//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.source;

import inet.queueing.base.PassivePacketSourceBase;
import inet.queueing.contract.IPassivePacketSource;

//
// A passive packet source which can be pulled for packets from
// the connected packet collector.
//
simple PassivePacketSource extends PassivePacketSourceBase like IPassivePacketSource
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        double initialProvidingOffset @unit(s) = default(0s); // Initial duration before which packets are not provided
        volatile double providingInterval @unit(s) = default(0s); // Elapsed time between subsequent packets allowed to be pulled by the connected packet collector, 0 means any number of packets can be pulled at the same simulation time
        bool scheduleForAbsoluteTime = default(true); // When a clock is used, relative means that setting the clock will not affect the simulation time of the event
        @class(PassivePacketSource);
    gates:
        output out @labels(pull);
}
