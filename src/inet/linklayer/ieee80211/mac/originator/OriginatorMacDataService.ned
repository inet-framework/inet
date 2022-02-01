//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.linklayer.ieee80211.mac.contract.IFragmentationPolicy;

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

