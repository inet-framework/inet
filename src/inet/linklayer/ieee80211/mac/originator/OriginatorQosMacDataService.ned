//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.linklayer.ieee80211.mac.contract.IFragmentationPolicy;
import inet.linklayer.ieee80211.mac.contract.IMsduAggregationPolicy;
import inet.linklayer.ieee80211.mac.contract.IMpduAggregationPolicy;

//
// Implements the MAC data service for the originator (sender) side in IEEE 802.11
// QoS networks. Extends the basic MAC data service with QoS capabilities including
// MSDU aggregation (A-MSDU) and MPDU aggregation (A-MPDU). Processes outgoing
// frames by assigning sequence numbers, performing aggregation, and fragmentation
// based on configured policies. Follows the MAC data plane architecture described
// in IEEE 802.11 standard.
//
module OriginatorQosMacDataService
{
    parameters:
        @class("OriginatorQosMacDataService");
        @display("i=block/fork");
        @signal[packetFragmented](type=inet::Packet);
        @signal[packetAggregated](type=inet::Packet);
        @statistic[packetFragmented](title="packets fragmented"; record=count);
        @statistic[packetAggregated](title="packets aggregated"; record=count);
    submodules:
        msduAggregationPolicy: <default("BasicMsduAggregationPolicy")> like IMsduAggregationPolicy if typename != "" {
            parameters:
                @display("p=100,100");
        }
        mpduAggregationPolicy: <default("BasicMpduAggregationPolicy")> like IMpduAggregationPolicy if typename != "" {
            parameters:
                @display("p=100,300");
        }
        fragmentationPolicy: <default("BasicFragmentationPolicy")> like IFragmentationPolicy if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

