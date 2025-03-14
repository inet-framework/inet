//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.rateselection;

//
// QoS-aware implementation of IEEE 802.11 rate selection for 802.11e/n/ac networks. 
// Extends the basic rate selection functionality with support for QoS frames and 
// features like Block Acknowledgment. Determines appropriate transmission rates for 
// different frame types including QoS data frames and specialized control frames 
// like Block ACK. Works in conjunction with QoS coordination functions to apply 
// appropriate rate selection policies in QoS-enabled networks.
//
// @see ~IRateSelection, ~RateSelection
//
simple QosRateSelection
{
    parameters:
        string rateControlModule;

        double multicastFrameBitrate @unit(bps) = default(-1bps);

        double responseAckFrameBitrate @unit(bps) = default(-1bps);
        double responseBlockAckFrameBitrate @unit(bps) = default(-1bps);
        double responseCtsFrameBitrate @unit(bps) = default(-1bps);

        double dataFrameBitrate @unit(bps) = default(-1bps); // Fastest
        double dataFrameBandwidth @unit(Hz) = default(nan Hz); // Unspecified by default
        int dataFrameNumSpatialStreams = default(-1); // Unspecified by default

        double mgmtFrameBitrate @unit(bps) = default(-1bps); // Fastest
        double controlFrameBitrate @unit(bps) = default(-1bps);
        @display("i=block/cogwheel");
}
