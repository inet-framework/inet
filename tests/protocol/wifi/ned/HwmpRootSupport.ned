// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.protocoltest.wifi;
import inet.linklayer.ieee80211.mesh.hwmp.Hwmp;
import inet.protocoltest.wifi.HwmpTestManagement;
simple HwmpRootProbe extends Hwmp { parameters: @class(::HwmpRootProbe); @lifecycleSupport; }
simple RootTestManagement extends HwmpTestManagement { parameters: @class(::RootTestManagement); @lifecycleSupport; }
