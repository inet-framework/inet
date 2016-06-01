//
// This program is property of its copyright holder. All rights reserved.
//

package inet.node.gpsr;

import inet.node.inet.WirelessHost;
import inet.routing.gpsr.GPSR;

module GPSRRouter extends WirelessHost
{
    parameters:
        @display("i=device/cellphone");
        forwarding = true;
        wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");

    submodules:
        gpsr: GPSR {
            parameters:
                routingTableModule = default("^.ipv4.routingTable");
                networkProtocolModule = default("^.ipv4.ip");
                @display("p=700,300");
        }

    connections:
        gpsr.ipOut --> tn.upperLayerIn++;
        gpsr.ipIn <-- tn.upperLayerOut++;
}
