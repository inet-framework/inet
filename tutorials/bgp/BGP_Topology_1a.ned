package inet.tutorials.bgp;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedMultiCanvasVisualizer;


network BGP_Topology_1a
{
    @display("bgb=1081.6675,985.49");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=93,44");
        }
        visualizer: IntegratedMultiCanvasVisualizer {
            @display("p=243.2025,43.536247");
        }
        RB2: Router {
            @display("p=770.3949,436.40866");
        }
        RB3: Router {
            @display("p=770.3949,525.4717");
        }
        RB4: Router {
            @display("p=877.2705,485.3933");
        }
        RB1: Router {
            @display("p=670.19904,485.3933");
        }
        RA4: Router {
            @display("p=407.4632,487.6199");
        }
        RA2: Router {
            @display("p=305.04077,427.50238");
        }
        RA3: Router {
            @display("p=305.04077,525.4717");
        }

        RA1: Router {
            @display("p=209.29803,478.7136");
        }
        switch1: EtherSwitch {
            @display("p=115.78189,476.48703");
        }
        switch2: EtherSwitch {
            @display("p=305.04077,333.98624");
        }
        switch3: EtherSwitch {
            @display("p=305.04077,618.9878");
        }
        switch4: EtherSwitch {
            @display("p=770.3949,333.98624");
        }
        switch5: EtherSwitch {
            @display("p=770.3949,618.9878");
        }
        switch6: EtherSwitch {
            @display("p=1004.18524,483.16675");
        }
        host0: StandardHost {
            @display("p=113.55532,333.98624");
        }
        host1: StandardHost {
            @display("p=1004.18524,347.34567");
        }
        RC1: Router {
            @display("p=425.2758,754.8089");
        }
        RC3: Router {
            @display("p=527.69824,797.11383");
        }
        RC4: Router {
            @display("p=630.12067,754.8089");
        }
        switch7: EtherSwitch {
            @display("p=761.4886,750.3557");
        }
        host2: StandardHost {
            @display("p=760.1975,862.96246");
        }
        switch8: EtherSwitch {
            @display("p=527.69824,888.4034");
        }
        RC2: Router {
            @display("p=527.69824,703.59766");
        }
        RD2: Router {
            @display("p=542.81,176.545");
        }
        RD3: Router {
            @display("p=542.81,266.135");
        }
        RD4: Router {
            @display("p=646.8925,225.2925");
        }
        switch9: EtherSwitch {
            @display("p=778.6425,223.97499");
        }
        RD1: Router {
            @display("p=442.68,225.2925");
        }
        switch10: EtherSwitch {
            @display("p=542.81,72.4625");
        }
        host3: StandardHost {
            @display("p=776.0075,115.939995");
        }
        switch11: EtherSwitch {
            @display("p=304.3425,221.34");
        }
        switch12: EtherSwitch {
            @display("p=304.3425,749.6575");
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
        RC2.ethg++ <--> Eth100M <--> RC4.ethg++;
        RC3.ethg++ <--> Eth100M <--> RC4.ethg++;
        RC1.ethg++ <--> Eth100M <--> RC2.ethg++;
        RC1.ethg++ <--> Eth100M <--> RC3.ethg++;
        RC3.ethg++ <--> Eth100M <--> switch8.ethg++;
        switch7.ethg++ <--> Eth100M <--> host2.ethg++;
        RC4.ethg++ <--> Eth100M <--> switch7.ethg++;
        RC2.ethg++ <--> Eth100M <--> RB1.ethg++;
        RC2.ethg++ <--> Eth100M <--> RA4.ethg++;
        RD2.ethg++ <--> Eth100M <--> RD4.ethg++;
        RD3.ethg++ <--> Eth100M <--> RD4.ethg++;
        RD1.ethg++ <--> Eth100M <--> RD2.ethg++;
        RD1.ethg++ <--> Eth100M <--> RD3.ethg++;
        switch9.ethg++ <--> Eth100M <--> host3.ethg++;
        RD4.ethg++ <--> Eth100M <--> switch9.ethg++;
        RB1.ethg++ <--> Eth100M <--> RD3.ethg++;
        RD1.ethg++ <--> Eth100M <--> switch11.ethg++;
        RD2.ethg++ <--> Eth100M <--> switch10.ethg++;
        RC1.ethg++ <--> Eth100M <--> switch12.ethg++;
        RA4.ethg++ <--> Eth100M <--> RD3.ethg++;
}

