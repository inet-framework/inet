//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.rateselection;

import inet.linklayer.ieee80211.mac.contract.IRateSelection;

//
// Default implementation of IEEE 802.11 rate selection that determines appropriate 
// transmission rates for different frame types. Applies configurable policies to select 
// bit rates for control frames (RTS/CTS/ACK), management frames, and data frames. 
// Works in conjunction with rate control algorithms to apply frame-specific rate 
// selection rules while respecting the overall rate adaptation strategy. Supports 
// configuration of different rates for unicast, multicast, and broadcast frames.
//
// @see ~IRateSelection, ~QosRateSelection
//
simple RateSelection like IRateSelection
{
    parameters:
        string rateControlModule;

        double multicastFrameBitrate @unit(bps) = default(-1bps);

        double responseAckFrameBitrate @unit(bps) = default(-1bps);
        double responseCtsFrameBitrate @unit(bps) = default(-1bps);

        double dataFrameBitrate @unit(bps) = default(-1bps); // Fastest
        double dataFrameBandwidth @unit(Hz) = default(nan Hz); // Unspecified by default
        int dataFrameNumSpatialStreams = default(-1); // Unspecified by default

        double mgmtFrameBitrate @unit(bps) = default(-1bps); // Fastest
        double controlFrameBitrate @unit(bps) = default(-1bps);
        @display("i=block/cogwheel");
}
