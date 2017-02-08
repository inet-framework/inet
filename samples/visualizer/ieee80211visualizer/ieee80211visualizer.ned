



package inet.samples.visualizer.ieee80211visualizer;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network Ieee80211visualizerA
{
    @display("bgb=746.852,436.192");
    submodules:
        accessPoint: AccessPoint {
            @display("p=420.97598,201.612");
        }
        wirelessHost: WirelessHost {
            @display("p=173.716,202.88");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=651.752,55.792");
        }
        visualizer: IntegratedVisualizer {
            @display("p=651.752,129.336");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=650.484,202.88");
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
                @display("p=300,50");
        }
        accessPointB: AccessPoint {
            parameters:
                @display("p=500,50");
        }
        hostA1: WirelessHost {
            parameters:
                @display("p=300,249");
        }
        hostA2: WirelessHost {
            parameters:
                @display("p=300,349");
        }
        hostB1: WirelessHost {
            parameters:
                @display("p=500,249");
        }
        hostB2: WirelessHost {
            parameters:
                @display("p=500,349");
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

network Ieee80211visualizerD
{
    @display("bgb=746.852,436.192");
    submodules:
        accessPoint: AccessPoint {
            @display("p=420.97598,201.612");
        }
        wirelessHost: WirelessHost {
            @display("p=173.716,202.88");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=651.752,55.792");
        }
        visualizer: IntegratedVisualizer {
            @display("p=651.752,129.336");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=650.484,202.88");
        }
}
