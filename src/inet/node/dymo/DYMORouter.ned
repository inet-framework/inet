//
// This program is property of its copyright holder. All rights reserved.
//

package inet.node.dymo;

import inet.node.inet.WirelessHost;
import inet.routing.dymo.IDYMO;


//
// TODO
//
module DYMORouter extends WirelessHost
{
    parameters:
        @display("i=device/cellphone");
        forwarding = true;
        wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");
        string dymoType = default("DYMO");
    submodules:
        dymo: <dymoType> like IDYMO {
            @display("p=527,287");
        }
    connections:
        dymo.ipOut --> networkLayer.transportIn++;
        dymo.ipIn <-- networkLayer.transportOut++;
}
