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


network VirtualLink_2
{
    @display("bgb=1421.8575,584.4563");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=136.23125,61.399998");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=136.05376,208.87126");
        }
        scenarioManager: ScenarioManager {
            @display("p=136.05376,360.255");
        }
        R3: Router {
            @display("p=703.2638,465.64877");
        }
        R1: Router {
            @display("p=904.47003,260.61002");
        }
        switch3: EtherSwitch {
            @display("p=448.40253,463.7325");
        }
        switch1: EtherSwitch {
            @display("p=904.47003,47.906254");
        }
        R2: Router {
            @display("p=1052.0212,465.64877");
        }
        switch2: EtherSwitch {
            @display("p=1306.8826,463.7325");
        }
    connections:
        R3.ethg++ <--> Eth100M <--> switch3.ethg++;
        R1.ethg++ <--> Eth100M <--> switch1.ethg++;
        R2.ethg++ <--> Eth100M <--> switch2.ethg++;
        R1.pppg++ <--> PppLink100M <--> R2.pppg++;
        R2.pppg++ <--> PppLink100M <--> R3.pppg++;
}

