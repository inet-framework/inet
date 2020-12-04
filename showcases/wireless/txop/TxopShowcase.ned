package inet.showcases.wireless.txop;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.common.IntegratedVisualizer;


network TxopShowcase
{
    @display("bgb=675,400");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,50");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=50,100");
        }
        visualizer: IntegratedVisualizer {
            @display("p=50,150");
        }
        host1: AdhocHost {
            @display("p=288,110;i=misc/smartphone");
        }
        host2: AdhocHost {
            @display("p=480,258;i=misc/smartphone");
        }
}
