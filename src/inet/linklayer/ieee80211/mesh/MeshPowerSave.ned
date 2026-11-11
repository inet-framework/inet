// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.linklayer.ieee80211.mesh;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mesh.contract.IMeshPowerSave;

// Peer power modes, buffering, awake windows and directional service periods.
// Awake means TRANSCEIVER. Ordinary recovery/lifetime remains MAC-owned.
simple MeshPowerSave extends SimpleModule like IMeshPowerSave
{
    parameters:
        @class(MeshPowerSave);
        @lifecycleSupport;
        bool enabled = default(false);
        double wakeMargin @unit(s) = default(0s); // Nonnegative addition to public switching lead plus one simulation tick
        int missingAckRetryLimit = default(7); // 0..100 actual terminating-MPDU retransmissions per shared ending episode; 7 is model policy
        string mibModule = default("^.mib");
        string managementModule = default("^.mgmt");
        string macModule = default("^.mac");
        string hcfModule = default("^.mac.hcf");
        @signal[meshAwakeRequirementChanged](type=bool);
        @signal[meshServicePeriodOutcome](type=object);
        @display("i=block/control");
}
