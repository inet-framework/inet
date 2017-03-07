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


module OriginatorQoSMacDataService
{
    parameters:
        string aMsduAggregationPolicyType = default("BasicMsduAggregationPolicy");
        string fragmentationPolicyType = default("BasicFragmentationPolicy");
        @class("OriginatorQoSMacDataService");
        @display("i=block/fork");
    submodules:
        msduAggregationPolicy: <aMsduAggregationPolicyType> like IMsduAggregationPolicy if aMsduAggregationPolicyType != "" {
            parameters:
                @display("p=150,100");
        }
        fragmentationPolicy: <fragmentationPolicyType> like IFragmentationPolicy if fragmentationPolicyType != "" {
            parameters:
                @display("p=150,200");
        }
}
