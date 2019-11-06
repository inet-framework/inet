package inet.showcases.general.fingerprint;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network FingerprintShowcase
{
    @display("bgb=67.5,40");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=5,5");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=5,10");
        }
        visualizer: IntegratedVisualizer {
            @display("p=5,15");
        }
        host1: AdhocHost {
            @display("p=28.8,11;i=misc/smartphone");
        }
        host2: AdhocHost {
            @display("p=48,25.8;i=misc/smartphone");
        }
}
