package inet.tutorials.bgp;

import inet.common.packet.recorder.PcapRecorder;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.canvas.integrated.IntegratedMultiCanvasVisualizer;


network Multihop_EBGP
{
    @display("bgb=690,303");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=93,44");
        }
        visualizer: IntegratedMultiCanvasVisualizer {
            @display("p=243.2025,43.536247");
        }
        routingTableRecorder: RoutingTableRecorder {
            @display("p=426,43");
        }
        pcapRecorder: PcapRecorder {
            @display("p=636,42");
        }
        RA: Router {
            @display("p=161.33,151.385");
        }
        RB: Router {
            @display("p=523.77,151.385");
        }
        R: Router {
            @display("p=344.76,151.385");
        }

        switch2: EthernetSwitch {
            @display("p=49.725,153.595");
        }
        switch1: EthernetSwitch {
            @display("p=635.375,150.28");
        }
        host0: StandardHost {
            @display("p=49,253");
        }
        host1: StandardHost {
            @display("p=635,253");
        }
    connections:
        RB.ethg++ <--> Eth100M <--> R.ethg++;
        RA.ethg++ <--> Eth100M <--> R.ethg++;
        switch2.ethg++ <--> Eth100M <--> RA.ethg++;
        RB.ethg++ <--> Eth100M <--> switch1.ethg++;
        host1.ethg++ <--> Eth100M <--> switch1.ethg++;
        host0.ethg++ <--> Eth100M <--> switch2.ethg++;
}
