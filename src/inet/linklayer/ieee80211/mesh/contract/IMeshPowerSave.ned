// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.linklayer.ieee80211.mesh.contract;

// Owns mesh power-mode policy; packet storage and the TSF stay with their owners.
moduleinterface IMeshPowerSave
{
    parameters:
        @display("i=block/control");
}
