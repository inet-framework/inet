



package inet.samples.visualizer.interfacetablevisualizer;

import inet.visualizer.integrated.IntegratedVisualizer;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;
import inet.visualizer.linklayer.InterfaceTableVisualizer;


network Simple
{
    @display("bgb=400,400");
    submodules:
        hostA: StandardHost {
            @display("p=92,203");
        }
        hostB: StandardHost {
            @display("p=233,203");
        }
        visualizer: IntegratedVisualizer {
            @display("p=328,38");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=328,93");
        }
    connections:
        hostA.ethg++ <--> Eth100M <--> hostB.ethg++;
}

network Complex
{
    @display("bgb=400,400");
}
    