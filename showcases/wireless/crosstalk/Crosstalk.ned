package inet.showcases.wireless.crosstalk;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211DimensionalRadioMedium;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;

network CrosstalkShowcaseBaseNetwork
{
    @display("bgb=640,360");
    submodules:
        host1: AdhocHost {
            @display("p=230,180");
        }
        host2: AdhocHost {
            @display("p=370,180");
        }
        host3: AdhocHost {
            @display("p=290,130");
        }
        host4: AdhocHost {
            @display("p=440,130");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,50;is=s");
        }
        visualizer: IntegratedVisualizer {
            @display("p=50,100;is=s");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=50,150;is=s");
        }
}

network CrosstalkShowcaseOneScalarRadioMediumModule extends CrosstalkShowcaseBaseNetwork
{
    submodules:
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=74,191");
        }
}

network CrosstalkShowcaseTwoScalarRadioMediumModules extends CrosstalkShowcaseBaseNetwork
{
    submodules:
        radioMedium1: Ieee80211ScalarRadioMedium {
            @display("p=74,191");
        }
        radioMedium2: Ieee80211ScalarRadioMedium {
            @display("p=74,255");
        }
}

network CrosstalkShowcasePartiallyOverlappingFrequencyBands extends CrosstalkShowcaseBaseNetwork
{
    submodules:
        radioMedium: Ieee80211DimensionalRadioMedium {
            @display("p=50,200;is=s");
        }
}
