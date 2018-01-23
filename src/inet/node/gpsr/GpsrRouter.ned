//
// This program is property of its copyright holder. All rights reserved.
//

package inet.node.gpsr;

import inet.node.inet.WirelessHost;
import inet.routing.gpsr.Gpsr;

module GpsrRouter extends WirelessHost
{
    parameters:
        @display("i=device/cellphone");
        forwarding = true;
        wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");

    submodules:
        gpsr: Gpsr {
            parameters:
                @display("p=700,300");
        }

    connections:
        gpsr.ipOut --> tn.in++;
        gpsr.ipIn <-- tn.out++;
}
