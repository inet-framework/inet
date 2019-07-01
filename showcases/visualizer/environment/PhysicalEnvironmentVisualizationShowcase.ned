package inet.showcases.visualizer.environment;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;

network PhysicalEnvironmentVisualizationShowcase
{
    parameters:
        @display("bgb=0,0");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        physicalEnvironment: PhysicalEnvironment {
            parameters:
                @display("p=100,150");
        }
}

//network PhysicalEnvironment2 extends PhysicalEnvironmentVisualizationShowcase
//{
//    submodules:
//        configurator: Ipv4NetworkConfigurator {
//            @display("p=51,106");
//        }
//        radioMedium: Ieee80211ScalarRadioMedium {
//            @display("p=145,100");
//        }
//        wirelessHost: WirelessHost {
//            @display("p=25,12.5");
//        }
//        accessPoint: AccessPoint {
//            @display("p=25,17.5");
//        }
//}
