//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketProcessorBase;

simple InterpacketGapInserter extends PacketProcessorBase
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        bool initialChannelBusy = default(false); // assume that channel was busy before the simulation started
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
