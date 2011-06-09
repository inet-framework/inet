//
// This program is property of its copyright holder. All rights reserved.
// 

package inet.examples.wireless.multiradio;

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
        router[useRouting ? 1 : 0]: Router {
            @display("p=255,168");
        }
    connections:
        accessPoint1.ethg++ <--> DatarateChannel {  datarate = 100Mbps; } <--> accessPoint2.ethg++ if useEthernetSwitching;
}
