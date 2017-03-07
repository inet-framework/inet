package inet.examples.visualization.networkroute;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.dymo.DYMORouter;
import inet.node.ethernet.Eth100M;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;

network StaticNetworkRouteVisualizationExample
{
    parameters:
        @display("bgb=1000,1000");
        @figure[description](type=text; pos=30,30; font=Arial,24; text="Demonstrates how to display static routes in a wired network.\nSource node continuously pings destination node.\nRoute is eventually displayed as a polyline.");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=900,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=900,150");
        }
        source: StandardHost {
            parameters:
                @display("p=100,900");
        }
        destination: StandardHost {
            parameters:
                @display("p=900,100");
        }
        node[8]: Router;
    connections:
        source.ethg++ <--> Eth100M <--> node[0].ethg++;
        node[0].ethg++ <--> Eth100M <--> node[1].ethg++;
        node[0].ethg++ <--> Eth100M <--> node[2].ethg++;
        node[1].ethg++ <--> Eth100M <--> node[3].ethg++;
        node[1].ethg++ <--> Eth100M <--> node[4].ethg++;
        node[4].ethg++ <--> Eth100M <--> node[5].ethg++;
        node[4].ethg++ <--> Eth100M <--> node[6].ethg++;
        node[5].ethg++ <--> Eth100M <--> node[7].ethg++;
        node[5].ethg++ <--> Eth100M <--> destination.ethg++;
}

network DynamicNetworkRouteVisualizationExample
{
    parameters:
        @display("bgb=1000,1000");
        @figure[description](type=text; pos=30,30; font=Arial,24; text="Demonstrates how to display dynamically changing routes in a wireless network.\nSource node continuously pings destination node.\nRoute is eventually displayed as a polyline, and it changes as nodes move around.");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=900,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=900,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=900,250");
        }
        source: DYMORouter {
            parameters:
                @display("p=100,900");
        }
        destination: DYMORouter {
            parameters:
                @display("p=900,100");
        }
        node[20]: DYMORouter {
            parameters:
                @display("r=,,#707070");
        }
}
