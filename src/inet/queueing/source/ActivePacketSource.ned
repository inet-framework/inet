//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.source;

import inet.queueing.base.ActivePacketSourceBase;
import inet.queueing.contract.IActivePacketSource;

//
// This module is an active packet source which pushes packets into the connected
// packet consumer.
//
simple ActivePacketSource extends ActivePacketSourceBase like IActivePacketSource
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        double initialProductionOffset @mutable @unit(s) = default(0s);
        volatile double productionInterval @unit(s); // elapsed time between subsequent packets pushed to the connected packet consumer
        bool scheduleForAbsoluteTime = default(true); // when a clock is used relative means that setting the clock will not affect the simulation time of the event
        @class(ActivePacketSource);
    gates:
        output out @labels(push);
}
