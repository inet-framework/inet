package inet.tutorials.bgp;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedMultiCanvasVisualizer;


network BGP_Topology_1
{
    @display("bgb=1050.615,594.405");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=93,44");
        }
        visualizer: IntegratedMultiCanvasVisualizer {
            @display("p=243.2025,43.536247");
        }
        RB2: Router {
            @display("p=710.955,349.65");
        }
        RB3: Router {
            @display("p=710.955,441.225");
        }
        RB4: Router {
            @display("p=815.85,399.6");
        }
        RB1: Router {
            @display("p=607.725,399.6");
        }
        RA4: Router {
            @display("p=349.79123,400.83374");
        }
        RA2: Router {
            @display("p=244.70375,342.285");
        }
        RA3: Router {
            @display("p=244.70375,442.86874");
        }

        RA1: Router {
            @display("p=153.1275,393.32748");
        }
        switch1: EtherSwitch {
            @display("p=54.045,391.82623");
        }
        switch2: EtherSwitch {
            @display("p=244.70375,250.70874");
        }
        switch3: EtherSwitch {
            @display("p=244.70375,534.445");
        }
        switch4: EtherSwitch {
            @display("p=710.955,249.75");
        }
        switch5: EtherSwitch {
            @display("p=710.955,532.8");
        }
        switch6: EtherSwitch {
            @display("p=944.055,397.935");
        }
        host0: StandardHost {
            @display("p=53.28,251.415");
        }
        host1: StandardHost {
            @display("p=944.055,261.405");
        }
    connections:
        RB2.ethg++ <--> Eth100M <--> RB4.ethg++;
        RB3.ethg++ <--> Eth100M <--> RB4.ethg++;
        RB1.ethg++ <--> Eth100M <--> RB2.ethg++;
        RB1.ethg++ <--> Eth100M <--> RB3.ethg++;
        RB1.ethg++ <--> Eth100M <--> RA4.ethg++;
        RA4.ethg++ <--> Eth100M <--> RA2.ethg++;
        RA3.ethg++ <--> Eth100M <--> RA4.ethg++;
        RA2.ethg++ <--> Eth100M <--> RA1.ethg++;
        RA1.ethg++ <--> Eth100M <--> RA3.ethg++;
        switch1.ethg++ <--> Eth100M <--> RA1.ethg++;
        RA2.ethg++ <--> Eth100M <--> switch2.ethg++;
        RA3.ethg++ <--> Eth100M <--> switch3.ethg++;
        RB2.ethg++ <--> Eth100M <--> switch4.ethg++;
        RB3.ethg++ <--> Eth100M <--> switch5.ethg++;
        switch1.ethg++ <--> Eth100M <--> host0.ethg++;
        switch6.ethg++ <--> Eth100M <--> host1.ethg++;
        RB4.ethg++ <--> Eth100M <--> switch6.ethg++;
}

