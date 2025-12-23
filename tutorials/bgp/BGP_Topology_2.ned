package inet.tutorials.bgp;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedMultiCanvasVisualizer;


network BGP_Topology_2
{
    @display("bgb=1238.76,594.405");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=93,44");
        }
        visualizer: IntegratedMultiCanvasVisualizer {
            @display("p=243.2025,43.536247");
        }
        RB2: Router {
            @display("p=699.3,174.825");
        }
        RC1: Router {
            @display("p=872.46,392.94");
        }
        RC2: Router {
            @display("p=964.035,344.655");
        }
        RC3: Router {
            @display("p=964.035,444.555");
        }

        RB1: Router {
            @display("p=491.175,174.825");
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
        RC4: Router {
            @display("p=1048.95,392.94");
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
            @display("p=964.035,246.42");
        }
        switch5: EtherSwitch {
            @display("p=964.035,527.805");
        }
        switch6: EtherSwitch {
            @display("p=1158.84,391.275");
        }
        host0: StandardHost {
            @display("p=53.28,251.415");
        }
        host1: StandardHost {
            @display("p=1158.84,261.405");
        }
    connections:
        RC1.ethg++ <--> Eth100M <--> RC2.ethg++;
        RC1.ethg++ <--> Eth100M <--> RC3.ethg++;
        RB2.ethg++ <--> Eth100M <--> RC1.ethg++;
        RB1.ethg++ <--> Eth100M <--> RA4.ethg++;
        RA4.ethg++ <--> Eth100M <--> RA2.ethg++;
        RA3.ethg++ <--> Eth100M <--> RA4.ethg++;
        RA2.ethg++ <--> Eth100M <--> RA1.ethg++;
        RA1.ethg++ <--> Eth100M <--> RA3.ethg++;
        RC4.ethg++ <--> Eth100M <--> RC2.ethg++;
        RC3.ethg++ <--> Eth100M <--> RC4.ethg++;
        switch1.ethg++ <--> Eth100M <--> RA1.ethg++;
        RA2.ethg++ <--> Eth100M <--> switch2.ethg++;
        RA3.ethg++ <--> Eth100M <--> switch3.ethg++;
        RC2.ethg++ <--> Eth100M <--> switch4.ethg++;
        RC3.ethg++ <--> Eth100M <--> switch5.ethg++;
        RC4.ethg++ <--> Eth100M <--> switch6.ethg++;
        switch1.ethg++ <--> Eth100M <--> host0.ethg++;
        switch6.ethg++ <--> Eth100M <--> host1.ethg++;
        RB2.ethg++ <--> Eth100M <--> RB1.ethg++;
}


