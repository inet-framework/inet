// SPDX-License-Identifier: LGPL-3.0-or-later

package inet.linklayer.ieee80211.mac.aggregation;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.IMpduAggregationPolicy;

// Selects independent HT20 long-GI members within the caller's negotiated receiver limits.
simple HtMpduAggregationPolicy extends SimpleModule like IMpduAggregationPolicy
{
    parameters:
        @class(HtMpduAggregationPolicy);
        @display("i=block/control");
}
