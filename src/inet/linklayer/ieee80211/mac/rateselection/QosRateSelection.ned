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

        double responseAckFrameBitrate @unit(bps) = default(-1bps);
        double responseBlockAckFrameBitrate @unit(bps) = default(-1bps);
        double responseCtsFrameBitrate @unit(bps) = default(-1bps);

        double dataFrameBitrate @unit(bps) = default(-1bps); // Fastest
        double dataFrameBandwidth @unit(Hz) = default(nan Hz); // Unspecified by default
        int dataFrameNumSpatialStreams = default(-1); // Unspecified by default

        // Per-receiver unicast data-frame rate. Keys are peer interface module paths (e.g.
        // "host1.wlan[0]"), resolved to MAC addresses at run time; values are bitrates (bps).
        // A unicast data frame whose receiver matches a key is transmitted at that rate;
        // unmatched receivers fall back to dataFrameBitrate / the rate-control module. The
        // bandwidth and spatial-stream count are taken from dataFrameBandwidth /
        // dataFrameNumSpatialStreams. Empty by default (capability inert).
        object dataFrameBitratePerReceiver = default({});

        double mgmtFrameBitrate @unit(bps) = default(-1bps); // Fastest
        double controlFrameBitrate @unit(bps) = default(-1bps);
        @display("i=block/cogwheel");
}

