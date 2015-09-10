//
// This program is property of its copyright holder. All rights reserved.
//

package inet.examples.wireless.wiredandwirelesshostswithap;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


network WiredAndWirelessHostsWithAP
{
    @display("bgb=503,221");
    submodules:
        wirelessHost1: WirelessHost {
            @display("p=62,69");
        }
        wiredHost1: StandardHost {
            @display("p=296,134");
        }
        wiredHost2: StandardHost {
            @display("p=412,70");
        }
        router: Router {
            @display("p=296,69");
        }
        accessPoint: AccessPoint {
            @display("p=205,69");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                assignDisjunctSubnetAddresses = false;
                @display("p=367,165");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=115,165");
        }
    connections:
        accessPoint.ethg++ <--> Eth100M <--> router.ethg++;
        wiredHost1.ethg++ <--> Eth100M <--> accessPoint.ethg++;
        wiredHost2.ethg++ <--> Eth100M <--> router.ethg++;
}
