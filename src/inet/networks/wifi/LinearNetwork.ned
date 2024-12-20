//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks.wifi;

import inet.networks.base.WirelessNetworkBase;
import inet.node.contract.INetworkNode;
import inet.node.wireless.AccessPoint;

network LinearNetwork extends WirelessNetworkBase
{
    submodules:
        client: <default("WirelessHost")> like INetworkNode {
            @display("p=350,200");
        }
        ap: AccessPoint {
            @display("p=550,200");
        }
        server: <default("WirelessHost")> like INetworkNode {
            @display("p=750,200");
        }
}

