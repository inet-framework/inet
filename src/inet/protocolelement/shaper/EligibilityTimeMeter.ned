//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.shaper;

import inet.queueing.base.PacketMeterBase;
import inet.queueing.contract.IPacketMeter;

//
// This module is a packet meter which measures the packet flow that is passing
// through and optionally attaches an ~EligibilityTimeTag to the packets. The
// tag contains the calculated simulation time when the packet becomes eligibile
// for transmission according to the asynchronous shaper algorithm.
//
// This module is part of the asynchronous shaper infrastructure.
//
// @see ~EligibilityTimeGate, ~EligibilityTimeFilter, ~EligibilityTimeQueue, ~EligibilityTimeTag
//
simple EligibilityTimeMeter extends PacketMeterBase like IPacketMeter
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        int packetOverheadLength @unit(b) = default(0b); // extra packet length taken into account, can be used to represent overhead added by lower layers
        double committedInformationRate @unit(bps); // committed information rate of the packet flow
        int committedBurstSize @unit(b); // committed burst size of the packet flow
        double maxResidenceTime @unit(s) = default(-1s); // maximum packet residence time measured from the packet arrival time to the meter module to the transmission eligiblity time, not used by default
        @class(EligibilityTimeMeter);
        @signal[tokensChanged](type=double);
        @statistic[numTokens](title="number of tokens"; source=tokensChanged; record=vector; unit=tk; interpolationmode=linear);
}
