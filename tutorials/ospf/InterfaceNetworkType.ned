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

network InterfaceNetworkType
{
    @display("bgb=2331.1575,1232.7675");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=867.888,89.376");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=1131.312,89.376");
        }
        scenarioManager: ScenarioManager {
            @display("p=1420.608,89.376");
        }
        host0: StandardHost {
            @display("p=96.31875,198.475");
        }
        host1: StandardHost {
            @display("p=96.401245,359.31375");
        }
        host2: StandardHost {
            @display("p=96.31875,539.96875");
        }
        R1: Router {
            @display("p=545.0625,356.9625");
        }
        R2: Router {
            @display("p=1001.952,359.85602");
        }
        R3: Router {
            @display("p=1453.536,357.504");
        }
        host3: StandardHost {
            @display("p=2095.632,199.92");
        }
        host4: StandardHost {
            @display("p=2095.632,359.85602");
        }
        host5: StandardHost {
            @display("p=2095.632,519.792");
        }
        switch1: EtherSwitch {
            @display("p=320.625,356.9625");
        }
        switch2: EtherSwitch {
            @display("p=1843.968,355.152");
        }
        host6: StandardHost {
            @display("p=96.401245,727.39124");
        }
        host7: StandardHost {
            @display("p=96.401245,885.13873");
        }
        host8: StandardHost {
            @display("p=96.401245,1095.4688");
        }
        R4: Router {
            @display("p=543.3525,885.13873");
        }
        R5: Router {
            @display("p=1001.952,886.704");
        }
        R6: Router {
            @display("p=1451.8612,885.13873");
        }
        host9: StandardHost {
            @display("p=2094.5361,712.785");
        }
        host10: StandardHost {
            @display("p=2094.5361,902.66626");
        }
        host11: StandardHost {
            @display("p=2094.5361,1095.4688");
        }
        switch3: EtherSwitch {
            @display("p=1843.3087,882.21747");
        }
        switch4: EtherSwitch {
            @display("p=318.41623,882.21747");
        }
    connections:
        host1.ethg++ <--> Eth100M <--> switch1.ethg++;
        host0.ethg++ <--> Eth100M <--> switch1.ethg++;
        host2.ethg++ <--> Eth100M <--> switch1.ethg++;
        switch1.ethg++ <--> Eth100M <--> R1.ethg++;

        switch2.ethg++ <--> Eth100M <--> host3.ethg++;
        switch2.ethg++ <--> Eth100M <--> host4.ethg++;
        switch2.ethg++ <--> Eth100M <--> host5.ethg++;

        R3.ethg++ <--> Eth100M <--> switch2.ethg++;
        host9.ethg++ <--> Eth100M <--> switch3.ethg++;
        host10.ethg++ <--> Eth100M <--> switch3.ethg++;
        host11.ethg++ <--> Eth100M <--> switch3.ethg++;
        switch3.ethg++ <--> Eth100M <--> R6.ethg++;

        switch4.ethg++ <--> Eth100M <--> host6.ethg++;
        switch4.ethg++ <--> Eth100M <--> host7.ethg++;
        switch4.ethg++ <--> Eth100M <--> host8.ethg++;

        R4.ethg++ <--> Eth100M <--> switch4.ethg++;
        R1.pppg++ <--> PppLink100M <--> R2.pppg++;
        R2.pppg++ <--> PppLink100M <--> R3.pppg++;
        R6.ethg++ <--> Eth100M <--> R5.ethg++;
        R5.ethg++ <--> Eth100M <--> R4.ethg++;
}

