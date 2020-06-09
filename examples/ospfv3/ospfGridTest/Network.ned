

package inet.examples.ospfv3.ospfGridTest;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth100M;
import inet.node.inet.Router;
import inet.visualizer.integrated.IntegratedVisualizer;


network GridNetwork
{
    int numHosts;

    @display("bgb=400,400");
    submodules:
        Router[numHosts*numHosts]: Router;
        configurator: Ipv4NetworkConfigurator {
            @display("p=0,0");
        }
        visualizer: IntegratedVisualizer {
            @display("p=0,10");
        }
        routingTableRecorder: RoutingTableRecorder {
            parameters:
                @display("p=0,20");
        }
    connections:

        for i=0..numHosts*numHosts-2 {
            Router[i].ethg++ <--> Eth100M <--> Router[i+1].ethg++;
        }
        for i=0..numHosts*numHosts-numHosts-1 {
            Router[i].ethg++ <--> Eth100M <--> Router[i+numHosts].ethg++;
        }
}

