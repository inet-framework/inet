package inet.tutorials.bgp;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedMultiCanvasVisualizer;


network BGP_Topology_4
{
    @display("bgb=1328.6062,594.495");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=93,44");
        }
        visualizer: IntegratedMultiCanvasVisualizer {
            @display("p=243.2025,43.536247");
        }
        RB2: Router {
            @display("p=660.55,132.11");
        }
        RB3: Router {
            @display("p=660.55,223.68625");
        }
        RB4: Router {
            @display("p=765.6375,181.65125");
        }
        RC1: Router {
            @display("p=974.3112,385.82123");
        }
        RC2: Router {
            @display("p=1065.8875,336.28");
        }
        RC3: Router {
            @display("p=1065.8875,435.3625");
        }

        RB1: Router {
            @display("p=556.96375,181.65125");
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
            @display("p=1151.4587,385.82123");
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
            @display("p=660.55,33.0275");
        }
        switch5: EtherSwitch {
            @display("p=660.55,315.26248");
        }
        switch6: EtherSwitch {
            @display("p=1065.8875,237.1975");
        }
        switch7: EtherSwitch {
            @display("p=1065.8875,519.4325");
        }
        switch8: EtherSwitch {
            @display("p=1261.0499,382.81876");
        }
        host0: StandardHost {
            @display("p=53.28,251.415");
        }
        host1: StandardHost {
            @display("p=1260.405,251.415");
        }
    connections:
        RB2.ethg++ <--> Eth100M <--> RB4.ethg++;
        RB3.ethg++ <--> Eth100M <--> RB4.ethg++;
        RC1.ethg++ <--> Eth100M <--> RC2.ethg++;
        RC1.ethg++ <--> Eth100M <--> RC3.ethg++;
        RB4.ethg++ <--> Eth100M <--> RC1.ethg++;
        RB1.ethg++ <--> Eth100M <--> RB2.ethg++;
        RB1.ethg++ <--> Eth100M <--> RB3.ethg++;
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
        RB2.ethg++ <--> Eth100M <--> switch4.ethg++;
        RB3.ethg++ <--> Eth100M <--> switch5.ethg++;
        RC2.ethg++ <--> Eth100M <--> switch6.ethg++;
        RC3.ethg++ <--> Eth100M <--> switch7.ethg++;
        RC4.ethg++ <--> Eth100M <--> switch8.ethg++;
        switch1.ethg++ <--> Eth100M <--> host0.ethg++;
        switch8.ethg++ <--> Eth100M <--> host1.ethg++;
}

