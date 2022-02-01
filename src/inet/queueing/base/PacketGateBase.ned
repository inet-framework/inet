//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// This is a base module for various packet gate modules.
//
// @see ~IPacketGate
//
simple PacketGateBase extends PacketFlowBase
{
    parameters:
        displayStringTextFormat = default("passed %p pk (%l)");
        double bitrate @unit(bps) = default(nan bps);
        int extraLength @unit(b) = default(0b);
        double extraDuration @unit(s) = default(0s);
        @class(PacketGateBase);
        @display("i=block/cogwheel");
        @signal[gateStateChanged](type=bool);
        @statistic[gateState](title="gate state"; type=enum; enum=CLOSED, OPEN; source=gateStateChanged; record=vector; interpolationmode=sample-hold);
}
