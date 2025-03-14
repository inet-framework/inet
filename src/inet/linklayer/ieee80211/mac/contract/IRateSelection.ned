//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11 frame rate selection modules that determine appropriate 
// transmission rates for different frame types. Rate selection modules apply specific 
// policies to choose bit rates (or MCS values) for control frames (RTS/CTS/ACK), 
// management frames, and data frames. These modules work in conjunction with rate 
// control algorithms but focus on immediate frame-by-frame decisions rather than 
// long-term adaptation.
//
// @see ~RateSelection, ~QosRateSelection
//
moduleinterface IRateSelection
{
    parameters:
        @display("i=block/cogwheel");
}
