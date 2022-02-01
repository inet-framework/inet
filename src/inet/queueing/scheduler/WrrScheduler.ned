//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.scheduler;

import inet.queueing.base.PacketSchedulerBase;
import inet.queueing.contract.IPacketScheduler;

//
// This module implements weighted round-robin scheduling.
//
// There is a weight associated with each input gate.
// Initially each input gate gets as many tokens as their
// weight. When a packet is requested then the first input
// gate is selected that has a packet and enough tokens,
// and one token is consumed. When there are waiting input
// packets, but there is no enough tokens to schedule one,
// then each token bucket is filled up.
//
simple WrrScheduler extends PacketSchedulerBase like IPacketScheduler
{
    parameters:
        string weights;
        @class(WrrScheduler);
}

