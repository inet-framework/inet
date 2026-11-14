// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.protocoltest.wifi;
import inet.linklayer.ieee80211.mgmt.Ieee80211MgmtMesh;
simple HwmpTestManagement extends Ieee80211MgmtMesh {
    parameters: bool suppressOwnPreq = default(false); @class(::HwmpTestManagement); @lifecycleSupport; @signal[hwmpTestHandoff](type=inet::Packet);
}
