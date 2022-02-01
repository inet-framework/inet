//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.sink;

import inet.queueing.base.ActivePacketSinkBase;
import inet.queueing.contract.IActivePacketSink;

//
// This module is an active packet sink which pulls packets from the connected
// packet provider. All pulled packets are counted and deleted.
//
simple ActivePacketSink extends ActivePacketSinkBase like IActivePacketSink
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        double initialCollectionOffset @unit(s) = default(0s);
        volatile double collectionInterval @unit(s); // elapsed time between subsequent packets pulled from the connected packet provider
        bool scheduleForAbsoluteTime = default(true); // when a clock is used relative means that setting the clock will not affect the simulation time of the event
        @class(ActivePacketSink);
    gates:
        input in @labels(pull);
}

