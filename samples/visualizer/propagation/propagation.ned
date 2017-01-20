



package inet.samples.visualizer.propagation;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network PropagationA
{
    @display("bgb=920,566");
    submodules:
        wirelessHost: WirelessHost {
            @display("p=441,272");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=761,99");
        }
        visualizer: IntegratedVisualizer {
            @display("p=764,163");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=768,223");
        }
        wirelessHost1: WirelessHost {
            @display("p=179,271");
        }
}
