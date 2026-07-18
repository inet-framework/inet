//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.backoff;

import inet.queueing.base.PacketDelayerBase;
import inet.queueing.contract.IPacketDelayer;

//
// Delays each packet by a truncated binary exponential backoff. The contention window doubles
// with the (re)transmission attempt carried in the packet's TransmissionAttemptReq tag (set by
// a retransmission element such as ~Resending): for attempt a the exponent is
// E = min(~minBackoffExponent + a, ~maxBackoffExponent), the window is CW = 2^E - 1, and the
// delay is a uniformly random number of slots in [0, CW] times ~slotTime. This is the CSMA-style
// contention backoff that a plain ~PacketDelayer with a fixed random delay cannot express.
//
simple ExponentialBackoff extends PacketDelayerBase like IPacketDelayer
{
    parameters:
        @class(ExponentialBackoff);
        double slotTime @unit(s) = default(20us); // duration of one backoff slot
        int minBackoffExponent = default(1); // backoff exponent for the first attempt (CW_min = 2^this - 1)
        int maxBackoffExponent = default(10); // the exponent is capped here (CW_max = 2^this - 1)
        @display("i=block/delay");
}
