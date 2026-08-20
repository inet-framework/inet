//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.rateselection;

import inet.common.SimpleModule;

//
// Implements rate selection for IEEE 802.11 QoS frames.
// Selects appropriate transmission rates for different frame types (data, management,
// control, response) based on configured parameters and dynamic conditions. Supports
// different bitrates for multicast frames, response frames (ACK, CTS, BlockAck), and
// can use a rate control module for dynamic rate adaptation.
//
// @see IRateControl
//
simple QosRateSelection extends SimpleModule
{
    parameters:
        @class(QosRateSelection);
        string rateControlModule;

        double multicastFrameBitrate @unit(bps) = default(-1bps);

        // -1 selects the standard-derived automatic response. In a 2.4 GHz HT profile,
        // a configured HT bitrate is an upper bound mapped to the highest mandatory
        // non-HT rate at or below it; it does not select an exact HT ACK PPDU.
        double responseAckFrameBitrate @unit(bps) = default(-1bps);
        // -1 selects the standard-derived automatic response. In a 2.4 GHz HT profile,
        // a configured HT bitrate is an upper bound mapped to the highest mandatory
        // non-HT rate at or below it; it does not select an exact HT BlockAck PPDU.
        double responseBlockAckFrameBitrate @unit(bps) = default(-1bps);
        // -1 selects the standard-derived automatic response. A configured HT value is a deliberate
        // model override translated only to HT-mixed while preserving the resolved configured mode's
        // MCS, bandwidth, NSS, and GI; it can therefore bypass IEEE 802.11-2024
        // 10.6.6.5.3/10.6.6.5.7 response constraints.
        double responseCtsFrameBitrate @unit(bps) = default(-1bps);

        double dataFrameBitrate @unit(bps) = default(-1bps); // Fastest
        double dataFrameBandwidth @unit(Hz) = default(nan Hz); // Unspecified by default
        int dataFrameNumSpatialStreams = default(-1); // Unspecified by default

        double mgmtFrameBitrate @unit(bps) = default(-1bps); // Fastest
        double controlFrameBitrate @unit(bps) = default(-1bps);
        @display("i=block/cogwheel");
}
