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
        gpsr4: GPSR if hasIpv4 {
            parameters:
                routingTableModule = "^.ipv4.routingTable";
                networkProtocolModule = "^.ipv4.ip";
                @display("p=700,300");
        }
        gpsr6: GPSR  if hasIpv6{
            parameters:
                routingTableModule = "^.ipv6.routingTable";
                networkProtocolModule = "^.ipv6.ipv6";
                @display("p=750,300");
        }
        gpsrGn: GPSR  if hasGn{
            parameters:
                routingTableModule = "^.gn.routingTable";
                networkProtocolModule = "^.gn.gnp";
                @display("p=800,300");
        }

    connections:
        gpsr4.ipOut --> tn.upperLayerIn++ if hasIpv4;
        gpsr4.ipIn <-- tn.upperLayerOut++ if hasIpv4;

        gpsr6.ipOut --> tn.upperLayerIn++ if hasIpv6;
        gpsr6.ipIn <-- tn.upperLayerOut++ if hasIpv6;

        gpsrGn.ipOut --> tn.upperLayerIn++ if hasGn;
        gpsrGn.ipIn <-- tn.upperLayerOut++ if hasGn;
}
