package inet.tutorials.ospf;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;
import inet.common.scenario.ScenarioManager;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth10M;
import ned.DatarateChannel;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

network OspfNetwork
{
    @display("bgb=2099.7375,852.525");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

        channel PppLink10M extends DatarateChannel
        {
            delay = 5us;
            datarate = 10Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=757.8,76.30625");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=1023.5563,76.30625");
        }
        scenarioManager: ScenarioManager {
            @display("p=1310.3625,76.30625");
        }
        host0: StandardHost {
            @display("p=97.81249,375.59998");
        }
        host1: StandardHost {
            @display("p=97.81249,532.1");
        }
        host2: StandardHost {
            @display("p=97.81249,694.46875");
        }
        host3: StandardHost {
            @display("p=1271.5625,334.51874");
        }
        R1: Router {
            @display("p=543.83746,532.1");
        }
        R3: Router {
            @display("p=1508.2687,532.1");
        }
        R2: Router {
            @display("p=925.3062,532.1");
        }
        host4: StandardHost {
            @display("p=1271.5625,692.51245");
        }
        host5: StandardHost {
            @display("p=1473.0562,389.29373");
        }
        host6: StandardHost {
            @display("p=1977.7687,373.64374");
        }
        host7: StandardHost {
            @display("p=1977.7687,532.1");
        }
        host8: StandardHost {
            @display("p=1977.7687,692.51245");
        }
        switch0: EtherSwitch {
            @display("p=318.86874,530.14374");
        }
        switch2: EtherSwitch {
            @display("p=1727.3687,530.14374");
        }
        switch1: EtherSwitch {
            @display("p=1271.5625,530.14374");
        }
        R4: Router {
            @display("p=1054.4187,334.51874");
        }
        R5: Router {
            @display("p=712.07495,334.51874");
        }

    connections:
        host2.ethg++ <--> Eth100M <--> switch0.ethg++;
        host1.ethg++ <--> Eth100M <--> switch0.ethg++;
        host0.ethg++ <--> Eth100M <--> switch0.ethg++;
        switch0.ethg++ <--> Eth100M <--> R1.ethg++;

        switch1.ethg++ <--> Eth100M <--> host3.ethg++;
        switch1.ethg++ <--> Eth100M <--> host4.ethg++;
        switch1.ethg++ <--> Eth100M <--> host5.ethg++;
        switch1.ethg++ <--> Eth100M <--> R3.ethg++;

        switch2.ethg++ <--> Eth100M <--> host6.ethg++;
        switch2.ethg++ <--> Eth100M <--> host7.ethg++;
        switch2.ethg++ <--> Eth100M <--> host8.ethg++;

        R3.ethg++ <--> Eth100M <--> switch2.ethg++;
        R1.pppg++ <--> PppLink10M <--> R2.pppg++;
        R2.ethg++ <--> Eth10M <--> switch1.ethg++;
        R1.pppg++ <--> PppLink100M <--> R5.pppg++;
        R5.pppg++ <--> PppLink100M <--> R4.pppg++;
        R4.ethg++ <--> Eth100M <--> switch1.ethg++;
}

