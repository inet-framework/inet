//
// This program is property of its copyright holder. All rights reserved.
//

package inet.node.dymo;

import inet.node.inet.WirelessHost;
import inet.routing.dymo.DYMO;

module DYMORouter extends WirelessHost
{
    parameters:
        @display("i=device/cellphone");
        bool hasIpv4Dymo = default(hasIpv4);
        bool hasIpv6Dymo = default(hasIpv6);
        bool hasGnDymo = default(hasGn);
        forwarding = true;
        wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");

    submodules:
        dymo4: DYMO if hasIpv4 && hasIpv4Dymo {
            parameters:
                routingTableModule = "^.ipv4.routingTable";
                networkProtocolModule = "^.ipv4.ip";
                @display("p=700,300");
        }
        dymo6: DYMO  if hasIpv6 && hasIpv6Dymo {
            parameters:
                routingTableModule = "^.ipv6.routingTable";
                networkProtocolModule = "^.ipv6.ipv6";
                @display("p=750,300");
        }
        dymoGn: DYMO  if hasGn && hasGnDymo {
            parameters:
                routingTableModule = "^.generic.routingTable";
                networkProtocolModule = "^.generic.gnp";
                @display("p=800,300");
        }

    connections:
        dymo4.ipOut --> tn.upperLayerIn++ if hasIpv4 && hasIpv4Dymo;
        dymo4.ipIn <-- tn.upperLayerOut++ if hasIpv4 && hasIpv4Dymo;

        dymo6.ipOut --> tn.upperLayerIn++ if hasIpv6 && hasIpv6Dymo;
        dymo6.ipIn <-- tn.upperLayerOut++ if hasIpv6 && hasIpv6Dymo;

        dymoGn.ipOut --> tn.upperLayerIn++ if hasGn && hasGnDymo;
        dymoGn.ipIn <-- tn.upperLayerOut++ if hasGn && hasGnDymo;
}
