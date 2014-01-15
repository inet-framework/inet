//
// This program is property of its copyright holder. All rights reserved.
// 

package inet.nodes.dymo;

import inet.networklayer.routing.dymo.IDYMO;
import inet.nodes.inet.WirelessHost;


//
// TODO
//
module DYMORouter extends WirelessHost
{
    parameters:
        @display("i=device/cellphone");
        IPForward = true;
        wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");
        string dymoType = default("xDYMO");
    submodules:
        dymo: <dymoType> like IDYMO {
            @display("p=527,287");
        }
    connections:
        dymo.ipOut --> networkLayer.transportIn++;
        dymo.ipIn <-- networkLayer.transportOut++;
}
