



package inet.samples.visualizer.ieee80211visualizer;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network Ieee80211visualizerA
{
    @display("bgb=497,331");
    submodules:
        accessPoint: AccessPoint {
            @display("p=287,156");
        }
        wirelessHost: WirelessHost {
            @display("p=127,157");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=409,32");
        }
        visualizer: IntegratedVisualizer {
            @display("p=409,78");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=408,123");
        }
}

network Ieee80211visualizerB
{
    @display("bgb=574,468");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=99,111");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,178");
        }
        accessPointA: AccessPoint {
            parameters:
                @display("p=259,49");
        }
        accessPointB: AccessPoint {
            parameters:
                @display("p=459,49");
        }
        hostA1: WirelessHost {
            parameters:
                @display("p=259,248");
        }
        hostA2: WirelessHost {
            parameters:
                @display("p=259,348");
        }
        hostB1: WirelessHost {
            parameters:
                @display("p=459,248");
        }
        hostB2: WirelessHost {
            parameters:
                @display("p=459,348");
        }
}

network Ieee80211visualizerC
{
    @display("bgb=561,411");
    submodules:
        accessPointA: AccessPoint {
            @display("p=100,250");
        }
        accessPointB: AccessPoint {
            @display("p=450,250");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=100,344");
        }
        visualizer: IntegratedVisualizer {
            @display("p=203,344");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=317,346");
        }
        wirelessHost: WirelessHost {
            @display("p=100,150");
        }
}
