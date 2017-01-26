



package inet.samples.visualizer.ieee80211visualizer;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network Ieee80211visualizerA
{
    @display("bgb=835,630");
    submodules:
        accessPoint: AccessPoint {
            @display("p=523,301");
        }
        wirelessHost: WirelessHost {
            @display("p=211,302");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=722,55");
        }
        visualizer: IntegratedVisualizer {
            @display("p=722,129");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=721,202");
        }
}

network Ieee80211visualizerB
{
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
                @display("p=300,50");
        }
        accessPointB: AccessPoint {
            parameters:
                @display("p=500,50");
        }
        hostA1: WirelessHost {
            parameters:
                @display("p=300,150");
        }
        hostA2: WirelessHost {
            parameters:
                @display("p=300,250");
        }
        hostB1: WirelessHost {
            parameters:
                @display("p=500,150");
        }
        hostB2: WirelessHost {
            parameters:
                @display("p=500,250");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=100,250");
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

network adhocinfrastructure
{
    @display("bgb=644,401");
    submodules:
        wirelessHost: WirelessHost {
            @display("p=153,201");
        }
        wirelessHost1: WirelessHost {
            @display("p=387,198");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=563,59");
        }
        visualizer: IntegratedVisualizer {
            @display("p=563,125");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=553,198");
        }
}
