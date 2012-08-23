//
// This program is property of its copyright holder. All rights reserved.
// 

package inet.examples.wireless.wiredandwirelesshostswithap;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.ethernet.Eth100M;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import inet.nodes.inet.WirelessHost;
import inet.nodes.wireless.AccessPoint;
import inet.world.radio.ChannelControl;


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
        iPv4NetworkConfigurator: IPv4NetworkConfigurator {
            parameters:
                assignDisjunctSubnetAddresses = false;
                @display("p=367,165");
        }
        channelControl: ChannelControl {
            @display("p=115,165");
        }
    connections:
        accessPoint.ethg++ <--> Eth100M <--> router.ethg++;
        wiredHost1.ethg++ <--> Eth100M <--> accessPoint.ethg++;
        wiredHost2.ethg++ <--> Eth100M <--> router.ethg++;
}
