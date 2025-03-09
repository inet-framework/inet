//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketProcessorBase;

//
// Inserts time gaps between consecutive packets to simulate interpacket delays
// in network transmissions. Ensures a minimum time separation between packets
// based on a configurable duration parameter.
//
// Operation:
// - Delays packet forwarding to maintain minimum time gaps between consecutive packets
// - Uses a timer to schedule packet processing after the required gap elapses
// - Supports both synchronous and asynchronous packet handling
// - Can be configured to assume the channel was busy before simulation started
//
// Particularly useful for:
// - Simulating realistic network transmission timing
// - Implementing rate limiting mechanisms
// - Modeling medium access control protocols
// - Emulating physical layer constraints in wireless networks
// - Testing protocol behavior with specific timing requirements
//
// @see ~PacketProcessorBase
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
