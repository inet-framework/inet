//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.inet;

//
// Models a host with (default) one wireless (802.11) card in infrastructure mode.
// This module is basically a ~StandardHost with an ~Ieee80211Interface with
// mgmt.typename = ~Ieee80211MgmtSta added.
// It should be used in conjunction with ~AccessPoint, or any other AP model
// which contains ~Ieee80211Interface with mgmt.typename = ~Ieee80211MgmtAp.
//
module WirelessHost extends StandardHost
{
    parameters:
        numWlanInterfaces = default(1);
        @display("i=device/wifilaptop");
}

