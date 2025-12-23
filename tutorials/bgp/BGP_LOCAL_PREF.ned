package inet.tutorials.bgp;

import inet.common.packet.recorder.PcapRecorder;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.visualizer.canvas.integrated.IntegratedMultiCanvasVisualizer;


network BGP_LOCAL_PREF
{
    @display("bgb=1328.6062,594.495");

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
        RB1: Router {
            @display("p=730.935,58.275");
        }
        RB2: Router {
            @display("p=730.935,362.97");
        }
        RB3: Router {
            @display("p=910.755,199.8");
        }
        RA: Router {
            @display("p=546.12,199.8");
        }
        switch1: EthernetSwitch {
            @display("p=366.3,198.135");
        }
        RC: Router {
            @display("p=564.435,512.82");
        }
        switch2: EthernetSwitch {
            @display("p=366.3,511.155");
        }
    connections:
        RB1.ethg++ <--> Eth100M <--> RB3.ethg++;
        RB2.ethg++ <--> Eth100M <--> RB3.ethg++;
        RA.ethg++ <--> Eth100M <--> RB1.ethg++;
        RA.ethg++ <--> Eth100M <--> RB2.ethg++;
        RC.ethg++ <--> Eth100M <--> RB2.ethg++;
        switch1.ethg++ <--> Eth100M <--> RA.ethg++;
        RC.ethg++ <--> Eth100M <--> switch2.ethg++;
}


