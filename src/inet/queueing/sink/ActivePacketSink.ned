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
// module. It pulls whole packets through the in gate at every collection interval.
// It pulls the first packet after the initial collection offset.
//
// If backpressure is present at the end of a collection interval, then it waits
// until the backpressure disappears, and then pulls in a packet. The collection
// interval timer always starts after the module pulls a packet in.
//
// All pulled packets are counted and deleted.
//
// Invariants:
//  - two subsequent packets are always pulled in at least collectionInterval time apart
//  - backpressure prevents packets from being pulled in
//
simple ActivePacketSink extends ActivePacketSinkBase like IActivePacketSink
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        double initialCollectionOffset @unit(s) = default(-1s); // initial duration before which packets are not collected, negative value means: try the first collection in initialize()
        volatile double collectionInterval @unit(s); // elapsed time between subsequent packets pulled from the connected packet provider
        bool scheduleForAbsoluteTime = default(true); // when a clock is used relative means that setting the clock will not affect the simulation time of the event
        @class(ActivePacketSink);
    gates:
        input in @labels(pull);
}

