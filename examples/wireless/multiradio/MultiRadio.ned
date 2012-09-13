//
// This program is property of its copyright holder. All rights reserved.
//

package inet.examples.wireless.multiradio;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.Router;
import inet.nodes.inet.WirelessHost;
import inet.nodes.wireless.AccessPoint;
import inet.world.radio.ChannelControl;
import ned.DatarateChannel;


network MultiRadio
{
    parameters:
        @display("bgb=505,274");
        bool useEthernetSwitching = default(true);
        bool useRouting = default(false);
    submodules:
        channelControl: ChannelControl {
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
        accessPoint2: AccessPoint {
            @display("p=354,99");
        }
        router: Router if useRouting {
            @display("p=255,168");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=187,34");
        }
    connections:
        accessPoint1.ethg++ <--> DatarateChannel {  datarate = 100Mbps; } <--> accessPoint2.ethg++ if useEthernetSwitching;
}
