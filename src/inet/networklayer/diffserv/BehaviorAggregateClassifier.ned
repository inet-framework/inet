//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.diffserv;

import inet.queueing.base.PacketClassifierBase;

//
// This module reads the DSCP (lower six bits of ToS/TrafficClass) from the
// received datagram, and forwards the datagram to the corresponding output gate.
//
// DSCPs are enumerated in the dscps parameter, first value is for out[0], second
// for out[1], etc. If the packet has no DSCP, or its value is not enumerated
// in the dscps parameter, it will be forwarded via the defaultOut gate.
//
simple BehaviorAggregateClassifier extends PacketClassifierBase
{
    parameters:
        string dscps = default(""); // space separated dscp values of the gates, both names (e.g. AF11, EF) and numbers (0x0A,0b101110) can be used
        @class(BehaviorAggregateClassifier);
        @signal[pkClass](type=long);
        @statistic[pkClass](title="packet class"; source=pkClass; record=vector; interpolationmode=none);
    gates:
        output defaultOut;
}

