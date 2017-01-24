



package inet.samples.visualizer.ieee80211visualizer;

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
    parameters:
        @figure[description](type=text; pos=300,10; font=Arial,20; text="TODO");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,150");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,250");
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
}

network Ieee80211visualizerC
{
    @display("bgb=561,411");
    submodules:
        accessPointA: AccessPoint {
            @display("p=92,176");
        }
        accessPointB: AccessPoint {
            @display("p=442,179");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=85,344");
        }
        visualizer: IntegratedVisualizer {
            @display("p=188,344");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=292,344");
        }
        wirelessHost: WirelessHost {
            @display("p=92,66");
        }
}

network Ieee80211visualizerD
{
    parameters:
        @figure[description](type=text; pos=300,10; font=Arial,20; text="TODO");
        @display("bgb=600,416");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,150");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,250");
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
        hostB3: WirelessHost {
            @display("p=300,356");
        }
        hostA3: WirelessHost {
            @display("p=500,356");
        }
}

network Ieee80211visualizerE
{
    parameters:
        @figure[description](type=text; pos=300,10; font=Arial,20; text="TODO");
        @display("bgb=811,493");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,150");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,250");
        }
        accessPointA: AccessPoint {
            parameters:
                @display("p=332,230");
        }
        accessPointB: AccessPoint {
            parameters:
                @display("p=519,230");
        }
        hostA1: WirelessHost {
            parameters:
                @display("p=266,130");
        }
        hostA2: WirelessHost {
            parameters:
                @display("p=220,231");
        }
        hostB1: WirelessHost {
            parameters:
                @display("p=573,130");
        }
        hostB2: WirelessHost {
            parameters:
                @display("p=634,231");
        }
        hostB3: WirelessHost {
            @display("p=266,329");
        }
        hostA3: WirelessHost {
            @display("p=573,329");
        }
}

network Ieee80211visualizerF
{
    parameters:
        @figure[description](type=text; pos=300,10; font=Arial,20; text="TODO");
        @display("bgb=811,493");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,150");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,250");
        }
        accessPointA: AccessPoint {
            parameters:
                @display("p=319.8875,130.4");
        }
        accessPointB: AccessPoint {
            parameters:
                @display("p=626.53125,129.38126");
        }
        hostA1: WirelessHost {
            parameters:
                @display("p=384.06876,213.9375");
        }
        hostA2: WirelessHost {
            parameters:
                @display("p=218.0125,213.9375");
        }
        hostB1: WirelessHost {
            parameters:
                @display("p=626.53125,254.6875");
        }
        hostB2: WirelessHost {
            parameters:
                @display("p=722.29376,213.9375");
        }
        hostB3: WirelessHost {
            @display("p=297.475,254.6875");
        }
        hostA3: WirelessHost {
            @display("p=521.6,213.9375");
        }
}
