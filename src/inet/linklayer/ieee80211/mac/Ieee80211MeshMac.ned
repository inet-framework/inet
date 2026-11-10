// SPDX-License-Identifier: LGPL-3.0-or-later

package inet.linklayer.ieee80211.mac;

// MAC boundary for the open mesh profile. Mesh data support is added separately.
module Ieee80211MeshMac extends Ieee80211Mac
{
    parameters:
        @class(Ieee80211MeshMac);
        string meshManagementModule = default("^.mgmt");
        qosStation = true;
}
