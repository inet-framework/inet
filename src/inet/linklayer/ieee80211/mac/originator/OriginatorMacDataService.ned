//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.linklayer.ieee80211.mac.contract.IFragmentationPolicy;

//
// Implements the IEEE 802.11 MAC data service for originator stations, responsible for 
// preparing MAC frames for transmission. This module processes outgoing frames by applying 
// fragmentation according to the configured policy. Fragmentation divides large frames 
// into smaller fragments to improve reliability in error-prone environments, as smaller 
// frames have a higher probability of successful transmission. Each fragment is transmitted 
// as an independent frame with its own MAC header and requires individual acknowledgment. 
// The module tracks statistics on fragmented and aggregated packets to monitor MAC 
// efficiency.
//
// @see ~RecipientMacDataService, ~OriginatorQosMacDataService
//
module OriginatorMacDataService
{
    parameters:
        @class("OriginatorMacDataService");
        @display("i=block/fork");
        @signal[packetFragmented](type=inet::Packet);
        @signal[packetAggregated](type=inet::Packet);
        @statistic[packetFragmented](title="packets fragmented"; record=count);
        @statistic[packetAggregated](title="packets aggregated"; record=count);
    submodules:
        fragmentationPolicy: <"BasicFragmentationPolicy"> like IFragmentationPolicy if typename != "" {
            parameters:
                @display("p=100,100");
        }
}
