//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.shaper;

import inet.queueing.base.PacketMeterBase;
import inet.queueing.contract.IPacketMeter;

//
// A packet meter which measures the packet flow that is passing
// through and optionally attaches an ~EligibilityTimeTag to the packets. The
// tag contains the calculated simulation time when the packet becomes eligible
// for transmission according to the asynchronous shaper algorithm.
//
// This module is part of the asynchronous shaper infrastructure.
//
// @see ~EligibilityTimeGate, ~EligibilityTimeFilter, ~EligibilityTimeQueue, ~EligibilityTimeTag
//
simple EligibilityTimeMeter extends PacketMeterBase like IPacketMeter
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        int packetOverheadLength @unit(b) = default(0b); // Extra packet length taken into account, can be used to represent overhead added by lower layers
        double committedInformationRate @unit(bps); // Committed information rate of the packet flow
        int committedBurstSize @unit(b); // Committed burst size of the packet flow
        double maxResidenceTime @unit(s) = default(-1s); // Maximum packet residence time measured from the packet arrival time to the meter module to the transmission eligibility time, not used by default
        displayStringTextFormat = default("contains {numTokens} tk\nbucket empty at {bucketEmptyTime}\nmetered %p pk (%l)");
        @class(EligibilityTimeMeter);
        @signal[tokensChanged](type=double);
        @statistic[numTokens](title="number of tokens"; source=tokensChanged; record=vector; unit=tk; interpolationmode=linear);
}
