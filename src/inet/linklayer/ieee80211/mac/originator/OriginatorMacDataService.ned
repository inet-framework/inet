//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.common.Module;
import inet.linklayer.ieee80211.mac.contract.IFragmentationPolicy;

//
// Implements the MAC data service for the originator (sender) side in non-QoS IEEE 802.11 
// networks. Processes outgoing frames by assigning sequence numbers and performing
// fragmentation based on the configured policy. Follows the MAC data plane
// architecture described in IEEE 802.11 standard to prepare frames for transmission.
//
// @see Dcf, OriginatorQosMacDataService
//
module OriginatorMacDataService extends Module
{
    parameters:
        @class(OriginatorMacDataService);
        @display("i=block/fork");
        @signal[packetFragmented](type=inet::Packet);
        @signal[packetAggregated](type=inet::Packet); // unused
        @statistic[packetFragmented](title="packets fragmented"; record=count);
        @statistic[packetAggregated](title="packets aggregated"; record=count); // unused
    submodules:
        fragmentationPolicy: <"BasicFragmentationPolicy"> like IFragmentationPolicy if typename != "" {
            parameters:
                @display("p=100,100");
        }
}

