// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.linklayer.ieee80211.mesh;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.IDs;

// Native endpoint mesh forwarding. Path selection is optional; without it,
// group propagation works and unresolved individual traffic is discarded.
simple MeshForwarding extends SimpleModule like IDs
{
    parameters:
        @class(MeshForwarding);
        @lifecycleSupport;
        string macModule = default("^");
        string mibModule;
        string pathSelectionModule = default("^.^.pathSelection");
        int initialTtl = default(31); // dot11MeshTTL
        bool forwarding = default(true); // dot11MeshForwarding
        int duplicateCacheCapacity = default(4096); // local bounded retention policy
        double duplicateCacheLifetime @unit(s) = default(1s); // local retention policy
        @signal[packetDropped];
        @display("i=block/routing");
}
