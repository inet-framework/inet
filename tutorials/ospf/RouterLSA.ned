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


network RouterLSA
{
    @display("bgb=1950.5325,964.7512");

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
            @display("p=1222.3688,86.74875");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=1461.585,86.74875");
        }
        scenarioManager: ScenarioManager {
            @display("p=1748.1188,86.74875");
        }
        host0: StandardHost {
            @display("p=144.58125,486.31876");
        }
        host1: StandardHost {
            @display("p=144.58125,646.6725");
        }
        host2: StandardHost {
            @display("p=144.58125,809.65497");
        }
        host3: StandardHost {
            @display("p=1069.9012,449.51624");
        }
        R1: Router {
            @display("p=586.21124,646.6725");
        }
        R2: Router {
            @display("p=1306.4888,646.6725");
        }
        host4: StandardHost {
            @display("p=1069.9012,807.02625");
        }
        host5: StandardHost {
            @display("p=1269.6863,504.72");
        }
        host6: StandardHost {
            @display("p=1774.4062,486.31876");
        }
        host7: StandardHost {
            @display("p=1774.4062,649.3013");
        }
        host8: StandardHost {
            @display("p=1774.4062,809.65497");
        }
        switch0: EtherSwitch {
            @display("p=365.39624,644.04376");
        }
        switch2: EtherSwitch {
            @display("p=1524.6749,644.04376");
        }
        switch1: EtherSwitch {
            @display("p=1069.9012,641.415");
        }
        R4: Router {
            @display("p=854.34375,449.51624");
        }
        R3: Router {
            @display("p=586.21124,449.51624");
        }

        host9: StandardHost {
            @display("p=775.48126,254.98875");
        }
        host10: StandardHost {
            @display("p=586.21124,86.74875");
        }
        host11: StandardHost {
            @display("p=365.39624,254.98875");
        }
        switch3: EtherSwitch {
            @display("p=586.21124,252.36");
        }
    connections:
        host2.ethg++ <--> Eth100M <--> switch0.ethg++;
        host1.ethg++ <--> Eth100M <--> switch0.ethg++;
        host0.ethg++ <--> Eth100M <--> switch0.ethg++;
        switch0.ethg++ <--> Eth100M <--> R1.ethg++;

        switch1.ethg++ <--> Eth100M <--> host3.ethg++;
        switch1.ethg++ <--> Eth100M <--> host4.ethg++;
        switch1.ethg++ <--> Eth100M <--> host5.ethg++;
        switch1.ethg++ <--> Eth100M <--> R2.ethg++;

        switch2.ethg++ <--> Eth100M <--> host6.ethg++;
        switch2.ethg++ <--> Eth100M <--> host7.ethg++;
        switch2.ethg++ <--> Eth100M <--> host8.ethg++;

        R2.ethg++ <--> Eth100M <--> switch2.ethg++;
        R1.pppg++ <--> PppLink100M <--> R3.pppg++;
        R4.ethg++ <--> Eth100M <--> switch1.ethg++;
        R1.ethg++ <--> Eth100M <--> switch1.ethg++;
        host10.ethg++ <--> Eth100M <--> switch3.ethg++;
        host11.ethg++ <--> Eth100M <--> switch3.ethg++;
        host9.ethg++ <--> Eth100M <--> switch3.ethg++;
        R3.ethg++ <--> Eth100M <--> switch3.ethg++;
}


