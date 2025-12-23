package inet.tutorials.bgp;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.Router;
import inet.visualizer.integrated.IntegratedMultiCanvasVisualizer;


network BGP_Basic_Topology
{
    @display("bgb=880.3688,269.73");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=93,44");
        }
        visualizer: IntegratedMultiCanvasVisualizer {
            @display("p=243.2025,43.536247");
        }
        RB2: Router {
            @display("p=817.9313,194.805");
        }
        RB1: Router {
            @display("p=642.69,194.805");
        }
        RA1: Router {
            @display("p=291.375,194.805");
        }
        RA2: Router {
            @display("p=141.10875,194.805");
        }

    connections:
        RB1.ethg++ <--> Eth100M <--> RB2.ethg++;
        RB1.ethg++ <--> Eth100M <--> RA1.ethg++;
        RA2.ethg++ <--> Eth100M <--> RA1.ethg++;
}


