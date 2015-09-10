//
// This program is property of its copyright holder. All rights reserved.
//

package inet.node.gpsr;

import inet.node.inet.WirelessHost;
import inet.routing.gpsr.IGPSR;


//
// TODO
//
module GPSRRouter extends WirelessHost
{
    parameters:
        @display("i=device/cellphone");
        forwarding = true;
        wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");
        string gpsrType = default("GPSR");
    submodules:
        gpsr: <gpsrType> like IGPSR {
            @display("p=527,287");
        }
    connections:
        gpsr.ipOut --> networkLayer.transportIn++;
        gpsr.ipIn <-- networkLayer.transportOut++;
}
