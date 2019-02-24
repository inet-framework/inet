//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.linklayer.ieee80211.mac.originator;

import inet.linklayer.ieee80211.mac.contract.IFragmentationPolicy;
import inet.linklayer.ieee80211.mac.contract.IMsduAggregationPolicy;
import inet.linklayer.ieee80211.mac.contract.IMpduAggregationPolicy;

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

