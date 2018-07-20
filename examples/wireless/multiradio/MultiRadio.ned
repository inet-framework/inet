//
// This program is property of its copyright holder. All rights reserved.
//

package inet.examples.wireless.multiradio;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import ned.DatarateChannel;

network MultiRadio
{
    parameters:
        bool useEthernetSwitching = default(true);
        bool useRouting = default(false);
    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            parameters:
                @display("p=100,300;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        host1: WirelessHost {
            parameters:
                @display("p=85,203");
        }
        host2: WirelessHost {
            parameters:
                @display("p=454,203");
        }
        accessPoint1: AccessPoint {
            parameters:
                @display("p=150,99");
        }
        accessPoint2: AccessPoint {
            parameters:
                @display("p=354,99");
        }
        router: Router if useRouting {
            parameters:
                @display("p=255,168");
        }
    connections:
        accessPoint1.ethg++ <--> DatarateChannel {  datarate = 100Mbps; } <--> accessPoint2.ethg++ if useEthernetSwitching;
}
