//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11n/ac MAC Protocol Data Unit (MPDU) aggregation policies 
// that determine when and how to combine multiple MPDUs into a single A-MPDU frame. 
// MPDU aggregation combines multiple complete MAC frames with their own MAC headers 
// into a single transmission opportunity, improving throughput by reducing overhead 
// and contention time. The aggregation policy decides which frames can be aggregated 
// together based on destination, traffic class, size constraints, and timing requirements.
//
// @see ~BasicMpduAggregationPolicy, ~IMsduAggregationPolicy
//
moduleinterface IMpduAggregationPolicy
{
    parameters:
        @display("i=block/control");
}
