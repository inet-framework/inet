//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.rateselection;

simple QosRateSelection
{
    parameters:
        string rateControlModule;

        double multicastFrameBitrate @unit(bps) = default(-1bps);

        double responseAckFrameBitrate @unit(bps) = default(-1bps);
        double responseBlockAckFrameBitrate @unit(bps) = default(-1bps);
        double responseCtsFrameBitrate @unit(bps) = default(-1bps);

        double dataFrameBitrate @unit(bps) = default(-1bps); // fastest
        double dataFrameBandwidth @unit(Hz) = default(nan Hz); // unspecified by default
        int dataFrameNumSpatialStreams = default(-1); // unspecified by default

        double mgmtFrameBitrate @unit(bps) = default(-1bps); // fastest
        double controlFrameBitrate @unit(bps) = default(-1bps);
        @display("i=block/cogwheel");
}

