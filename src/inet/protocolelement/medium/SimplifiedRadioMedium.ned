//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.medium;

import inet.common.SimpleModule;

//
// A minimal shared wireless medium. A frame transmitted on one ~radio gate is broadcast to every
// other in-range radio after the connecting channel's propagation delay. Radios connect via
// DatarateChannels (the medium sets them to MULTI mode so concurrent transmissions are allowed).
//
// Two simplifications on top of a plain broadcast bus:
//  - ~ranges limits which radios hear each other (so a line topology forces multi-hop forwarding);
//  - ~packetLossProbability drops in-range deliveries, driving retransmission + backoff.
//
// It does NOT model SNR/capture or collisions (concurrent transmissions are both delivered) -- that
// is a deliberate simplification; it is not INET's real RadioMedium.
//
simple SimplifiedRadioMedium extends SimpleModule
{
    parameters:
        @class(SimplifiedRadioMedium);
        @wireJunction;
        string ranges = default("");               // "i j; k l; ..." symmetric adjacency; empty = fully connected
        double packetLossProbability = default(0);  // per in-range delivery; reproducible under the fixed seed
        @display("i=misc/sun");
        @signal[packetReceived](type=cPacket);
        @statistic[packetReceived](title="packets"; source=packetReceived; record=count,"sum(packetBytes)"; interpolationmode=none);
    gates:
        inout radio[];
}
