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


network OSPF_Summary_LSA
{
    @display("bgb=1642.8824,629.9775");

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
            @display("p=210.055,102.585");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=473.845,102.585");
        }
        scenarioManager: ScenarioManager {
            @display("p=744.96246,102.585");
        }
        R1: Router {
            @display("p=564.0975,282.04874");
        }
        R2: Router {
            @display("p=174.99374,282.04874");
        }
        R3: Router {
            @display("p=1472.0062,282.04874");
        }
        R4: Router {
            @display("p=1084.9613,282.04874");
        }
        R1A: Router {
            @display("p=564.0975,516.7463");
        }
        R2A: Router {
            @display("p=174.99374,516.7463");
        }
        R3A: Router {
            @display("p=1472.0062,516.7463");
        }
        R4A: Router {
            @display("p=1084.9613,516.7463");
        }
    connections:
        R1.ethg++ <--> Eth100M <--> R2.ethg++;
        R1.ethg++ <--> Eth100M <--> R4.ethg++;
        R4.ethg++ <--> Eth100M <--> R3.ethg++;
        R2A.ethg++ <--> Eth100M <--> R2.ethg++;
        R1A.ethg++ <--> Eth100M <--> R1.ethg++;
        R4A.ethg++ <--> Eth100M <--> R4.ethg++;
        R3A.ethg++ <--> Eth100M <--> R3.ethg++;
}

