package inet.examples.visualization.routingtable;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.dymo.DYMORouter;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;

network RoutingTableVisualizationExample
{
    parameters:
        @figure[description](type=text; pos=300,10; font=Arial,20; text="TODO");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,250");
        }
        source: DYMORouter {
            parameters:
                @display("p=100,900");
        }
        destination: DYMORouter {
            parameters:
                @display("p=900,300");
        }
        node[20]: DYMORouter {
            parameters:
                @display("r=,,#707070");
        }
}
