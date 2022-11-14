//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.sink;

import inet.queueing.base.PassivePacketSinkBase;
import inet.queueing.contract.IPassivePacketSink;

//
// This module is a passive packet sink which is pushed with packets by the
// connected packet producer. All pushed packets are counted and deleted.
//
simple PassivePacketSink extends PassivePacketSinkBase like IPassivePacketSink
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        double initialConsumptionOffset @unit(s) = default(0s); // initial duration before which packets are not consumed
        volatile double consumptionInterval @unit(s) = default(0s); // elapsed time between subsequent packets allowed to be pushed by the connected packet producer, 0 means any number of packets can be pushed at the same simulation time
        bool scheduleForAbsoluteTime = default(true); // when a clock is used relative means that setting the clock will not affect the simulation time of the event
        @class(PassivePacketSink);
    gates:
        input in @labels(push);
}

