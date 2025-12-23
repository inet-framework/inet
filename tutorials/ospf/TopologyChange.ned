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


network TopologyChange
{
    @display("bgb=1914.2025,894.73376");

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
            @display("p=652.46,105.545");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=918.72125,105.545");
        }
        scenarioManager: ScenarioManager {
            @display("p=1204.1725,105.545");
        }
        R2: Router {
            @display("p=959.4937,573.0675");
        }
        switch3: EtherSwitch {
            @display("p=160.35374,570.4387");
        }
        R4: Router {
            @display("p=410.085,573.0675");
        }
        R3: Router {
            @display("p=709.7625,573.0675");
        }

        switch2: EtherSwitch {
            @display("p=1264.4287,570.4387");
        }
        switch5: EtherSwitch {
            @display("p=709.7625,378.54");
        }
        R1: Router {
            @display("p=1519.4175,573.0675");
        }
        switch4: EtherSwitch {
            @display("p=959.4937,378.54");
        }
        switch1: EtherSwitch {
            @display("p=1782.2925,567.81");
        }
        R5: Router {
            @display("p=1264.1412,379.0025");
        }
        R6: Router {
            @display("p=1264.1412,801.1825");
        }
        switch6: EtherSwitch {
            @display("p=1517.6176,376.4075");
        }
        switch7: EtherSwitch {
            @display("p=1517.6176,798.3675");
        }
    connections:
        R2.pppg++ <--> PppLink100M <--> R3.pppg++;
        R3.pppg++ <--> PppLink100M <--> R4.pppg++;
        R4.ethg++ <--> Eth100M <--> switch3.ethg++;
        R3.ethg++ <--> Eth100M <--> switch5.ethg++;
        R2.ethg++ <--> Eth100M <--> switch2.ethg++;
        R1.ethg++ <--> Eth100M <--> switch2.ethg++;
        switch4.ethg++ <--> Eth100M <--> R2.ethg++;
        switch1.ethg++ <--> Eth100M <--> R1.ethg++;
        switch2.ethg++ <--> Eth100M <--> R5.ethg++;
        R6.ethg++ <--> Eth100M <--> switch2.ethg++;
        switch6.ethg++ <--> Eth100M <--> R5.ethg++;
        switch7.ethg++ <--> Eth100M <--> R6.ethg++;
}


