//
// This program is property of its copyright holder. All rights reserved.
// 

package inet.nodes.gpsr;

import inet.networklayer.routing.gpsr.IGPSR;
import inet.nodes.inet.WirelessHost;

//
// TODO
//
module GPSRRouter extends WirelessHost
{
    parameters:
        @display("i=device/cellphone");
        IPForward = true;
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
