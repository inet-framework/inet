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

package inet.linklayer.ieee80211.mac.aggregation;

import inet.linklayer.ieee80211.mac.contract.IMsduAggregationPolicy;

simple BasicMsduAggregationPolicy like IMsduAggregationPolicy
{
    int subframeNumThreshold = default(-1); // Minimum number of subframes needed to compose an A-MSDU. (-1 means it won't be checked)
    int aggregationLengthThreshold = default(-1); // The minimum length for an aggregated payload necessary to create an A-MSDU (-1 means it won't be checked).
    int maxAMsduSize = default(4065); // -1 means infinity
    bool qOsCheck = default(true); // Turns on/off the check of the presence of the QoS flag (false means off -- this is needed for aggregation in DCF mode).
    @display("i=block/control");
}

