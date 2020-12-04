
package inet.showcases.wireless.fragmentation;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.common.IntegratedVisualizer;


network FragmentationShowcase
{
    @display("bgb=400,400");
    submodules:
        wifiHost1: AdhocHost {
            @display("p=215,99;i=misc/smartphone");
        }
        wifiHost2: AdhocHost {
            @display("p=315,283;i=misc/smartphone");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=78,50");
        }
        visualizer: IntegratedVisualizer {
            @display("p=78,130");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=78,205");
        }
}

