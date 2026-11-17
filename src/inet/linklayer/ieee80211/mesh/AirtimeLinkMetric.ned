// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.linklayer.ieee80211.mesh;
import inet.common.SimpleModule;

// One outgoing airtime estimator per mesh interface. Mode and access timing
// come from HCF; the MIB selects metric ID1 or2 for this mesh BSS.
simple AirtimeLinkMetric extends SimpleModule
{
    parameters:
        @class(AirtimeLinkMetric);
        @lifecycleSupport;
        string mibModule;
        string hcfModule = default("^.mac.hcf");
        @display("i=block/table");
}
