package inet.tutorials.bgp;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;


network BGP_Basic_Topology
{
    @display("bgb=880.3688,269.73");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=93,44");
        }
        visualizer: IntegratedCanvasVisualizer {
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

network BGP_Topology_1
{
    @display("bgb=1050.615,594.405");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=93,44");
        }
        visualizer: IntegratedCanvasVisualizer {
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

network BGP_Topology_1a
{
    @display("bgb=1081.6675,985.49");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=93,44");
        }
        visualizer: IntegratedCanvasVisualizer {
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

network BGP_Topology_2
{
    @display("bgb=1238.76,594.405");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=93,44");
        }
        visualizer: IntegratedCanvasVisualizer {
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

network BGP_Topology_3
{
    @display("bgb=1238.76,594.405");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=93,44");
        }
        visualizer: IntegratedCanvasVisualizer {
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
        RB3: Router {
            @display("p=599.265,97.8075");
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
        RB1.ethg++ <--> Eth100M <--> RB3.ethg++;
        RB3.ethg++ <--> Eth100M <--> RB2.ethg++;
}

network BGP_Topology_4
{
    @display("bgb=1328.6062,594.495");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=93,44");
        }
        visualizer: IntegratedCanvasVisualizer {
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

network BGP_LOCAL_PREF
{
    @display("bgb=1328.6062,594.495");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=93,44");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=243.2025,43.536247");
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
        switch1: EtherSwitch {
            @display("p=366.3,198.135");
        }
        RC: Router {
            @display("p=564.435,512.82");
        }
        switch2: EtherSwitch {
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

network Multihop_EBGP
{
    @display("bgb=690,303");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=93,44");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=243.2025,43.536247");
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

        switch2: EtherSwitch {
            @display("p=49.725,153.595");
        }
        switch1: EtherSwitch {
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
