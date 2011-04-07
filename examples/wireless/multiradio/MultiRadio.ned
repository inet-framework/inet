//
// This program is property of its copyright holder. All rights reserved.
// 

package inet.examples.wireless.multiradio;

// numOfHosts: 3

import inet.nodes.wireless.WirelessHost;
import inet.nodes.inet.Router;
import inet.world.ChannelControlExtended;
import inet.nodes.wireless.AccessPoint;
import inet.nodes.inet.StandardHost;
import inet.networklayer.autorouting.ipv4.FlatNetworkConfigurator;


network MultiRadio
{
    parameters:

        @display("bgb=505,274");
    submodules:
        channelControl: ChannelControlExtended {
            @display("p=53,32");
        }
        host1: WirelessHost {
            @display("p=85,203");
        }
        host2: WirelessHost {
            @display("p=454,203");
        }

        accessPoint1: AccessPoint {
            @display("p=150,99");
        }
//        accessPoint2: AccessPoint {
//
//            @display("p=354,99");
//        }
        configurator: FlatNetworkConfigurator {
            networkAddress = "192.168.1.0";
            netmask = "255.255.255.0";
            @display("p=150,32");
        }
//        router: Router {
//            @display("p=255,168");
//        }
}
