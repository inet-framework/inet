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
// Implements the IEEE 802.11e/n/ac QoS MAC data service for originator stations, extending 
// the basic MAC data service with Quality of Service capabilities. This module processes 
// outgoing frames by applying frame aggregation and fragmentation according to configured 
// policies. It supports two types of aggregation: A-MSDU (Aggregated MAC Service Data Unit) 
// combines multiple higher-layer packets into a single MAC frame, while A-MPDU (Aggregated 
// MAC Protocol Data Unit) combines multiple MAC frames into a single PHY transmission. 
// These aggregation techniques significantly improve throughput by reducing overhead and 
// contention time. The module also supports fragmentation for large frames and tracks 
// statistics on fragmented and aggregated packets.
//
// @see ~OriginatorMacDataService, ~RecipientQosMacDataService
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
