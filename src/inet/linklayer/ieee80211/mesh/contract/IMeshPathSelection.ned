// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.linklayer.ieee80211.mesh.contract;

// Supplies validated next hops and owns forwarding/precursor lifetimes.
moduleinterface IMeshPathSelection
{
    parameters:
        @display("i=block/routing");
}
