//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11n/ac MAC Service Data Unit (MSDU) aggregation policies 
// that determine when and how to combine multiple MSDUs into a single A-MSDU frame. 
// MSDU aggregation combines multiple higher-layer packets destined for the same 
// receiver into a single MAC frame with a shared MAC header, reducing overhead and 
// improving efficiency. The aggregation policy decides which frames can be aggregated 
// based on destination, traffic class, size constraints, and QoS requirements.
//
// @see ~BasicMsduAggregationPolicy, ~IMpduAggregationPolicy
//
moduleinterface IMsduAggregationPolicy
{
    parameters:
        @display("i=block/control");
}
