//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketProcessorBase;

//
// Enforces a minimum time gap between consecutive packets, which is important
// for protocols like Ethernet that require an interpacket gap (IPG) or
// interframe spacing. This module delays packet transmission to ensure the
// configured minimum gap duration is maintained between packets. It can
// optionally use an external clock module for timing, and supports both
// synchronous and asynchronous packet processing. The module tracks packet
// start and end times, emits signals when interpacket gaps start and end, and
// supports streaming operations with packet progress updates.
//
simple InterpacketGapInserter extends PacketProcessorBase
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        bool initialChannelBusy = default(false); // Assume that the channel was busy before the simulation started
        volatile double duration @unit(s);
        displayStringTextFormat = default("delayed %p pk (%l)\nifg: %g");
        @display("i=block/timer");
        @class(InterpacketGapInserter);
        @signal[interpacketGapStarted](type=double);
        @signal[interpacketGapEnded](type=double);
        @statistic[interpacketGap](title="interpacket gap state"; type=int; source=count(interpacketGapStarted) - count(interpacketGapEnded); record=vector; interpolationmode=sample-hold);
    gates:
        input in;
        output out;
}
