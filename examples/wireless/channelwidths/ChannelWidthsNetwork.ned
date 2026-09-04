//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.wireless.channelwidths;

import inet.networks.base.WirelessNetworkBase;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;

network ChannelWidthsNetwork extends WirelessNetworkBase
{
    parameters:
        @display("bgb=650,450");
    submodules:
        ap: AccessPoint {
            @display("p=250,200;r=,,#707070");
            wlan[*].mgmt.typename = "Ieee80211MgmtApSimplified";
        }
        sta1: WirelessHost {
            @display("p=400,150;r=,,#707070");
            wlan[*].mgmt.typename = "Ieee80211MgmtStaSimplified";
            wlan[*].agent.typename = "";
        }
        sta2: WirelessHost {
            @display("p=400,250;r=,,#707070");
            wlan[*].mgmt.typename = "Ieee80211MgmtStaSimplified";
            wlan[*].agent.typename = "";
        }
        interferer: WirelessHost {
            @display("p=400,350;r=,,#707070");
            wlan[*].mgmt.typename = "Ieee80211MgmtStaSimplified";
            wlan[*].agent.typename = "";
        }
}
