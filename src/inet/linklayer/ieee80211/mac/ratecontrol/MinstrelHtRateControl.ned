//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee80211.mac.ratecontrol;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.IRateControl;

//
// Minstrel-HT rate adaptation for HT and VHT normal-ACK data frames, following
// Linux v4.19 net/mac80211/rc80211_minstrel{,_ht}.c. Uses per-receiver/per-mode
// success probabilities, 75% historical-weight EWMA, airtime throughput ranking,
// randomized group sampling and a packet-specific multirate retry chain.
// Configure rateSelection.dataFrameBitrate = -1bps to enable adaptation.
//
// HT candidates use the existing negotiated peer MCS/width/GI constraints.
// VHT negotiation is not implemented by this checkout: configure maxChannelWidth
// and maxNumSpatialStreams for the common capabilities of the VHT peers.
// Local antenna count always limits spatial streams. Only catalog modes are used.
// Management and group traffic use a mandatory legacy rate and do not train it.
//
// The feedback API supplies individual normal-ACK attempts, so aggregation length
// is one. Block/no-ACK unicast data is rejected. Linux aggregation-size control,
// CCK fallback, SMPS and forced RTS on fallback stages are not modeled. Durations
// come from INET PHY modes, including preamble and ACK, instead of Linux's
// amortized duration tables. MAC retry limits can truncate the three-stage chain;
// after reaching the final stage its mode is retained until MAC recovery stops.
// getRate(address) reports the best throughput rate without consuming a probe;
// the MAC uses getRateForFrame(packet) to select and retain the actual retry plan.
//
simple MinstrelHtRateControl extends SimpleModule like IRateControl
{
    parameters:
        @class(MinstrelHtRateControl);
        string mibModule = default("^.^.^.mib");
        string radioModule = default("^.^.^.radio");
        double interval @unit(s) = default(50ms); // positive statistics update interval
        int packetLength @unit(B) = default(1200B); // positive reference MPDU length for airtime ranking
        double retrySegmentDuration @unit(s) = default(6ms); // positive per-stage airtime budget
        int maxRetryCount = default(7); // cap for sizing retry stages, in [2, 64]; MAC recovery owns termination
        double maxChannelWidth @unit(Hz) = default(20MHz); // positive common operational width ceiling
        int maxNumSpatialStreams = default(-1); // -1 uses local antenna count; otherwise a positive peer ceiling
        bool probeEnabled = default(true); // disable only for controlled fixed-candidate experiments
        displayStringTextFormat = default("{numStations} stations");
        @display("i=block/cogwheel");
        @signal[datarateChanged];
        @statistic[datarateChanged](title="best throughput data rate"; unit=bps; record=vector; interpolationmode=sample-hold);
        @statistic[dataratePerStation](title="best throughput data rate per station"; source=demux(datarateChanged); unit=bps; record=vector; interpolationmode=sample-hold);
}
