// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.linklayer.ieee80211.mesh;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mesh.contract.IMeshPowerSave;

// Peer power modes, buffering and TIM policy. Activation is gated until the
// complete mesh service-period engine is available. Awake means TRANSCEIVER.
simple MeshPowerSave extends SimpleModule like IMeshPowerSave
{
    parameters:
        @class(MeshPowerSave);
        @lifecycleSupport;
        bool enabled = default(false);
        double wakeMargin @unit(s) = default(0s); // Nonnegative addition to public switching lead plus one simulation tick
        string mibModule = default("^.mib");
        string managementModule = default("^.mgmt");
        string macModule = default("^.mac");
        string hcfModule = default("^.mac.hcf");
        @signal[meshAwakeRequirementChanged](type=bool);
        @display("i=block/control");
}
