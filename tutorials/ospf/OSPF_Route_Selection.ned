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


network OSPF_Route_Selection
{
    @display("bgb=506.016,497.98398");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=65,38");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=187,38");
        }
        scenarioManager: ScenarioManager {
            @display("p=324,38");
        }
        R1: Router {
            @display("p=447.784,290.156");
        }
        R2: Router {
            @display("p=236.944,140.56");
        }
        R3: Router {
            @display("p=236.944,240.95999");
        }
        R4: Router {
            @display("p=236.944,343.36798");
        }
        R5: Router {
            @display("p=236.944,448.788");
        }
        switch2: EtherSwitch {
            @display("p=64.256,137.54799");
        }
        switch4: EtherSwitch {
            @display("p=64.256,340.356");
        }
        switch5: EtherSwitch {
            @display("p=64.256,445.776");
        }
        R6: Router {
            @display("p=64.256,240.95999");
        }
    connections:
        R2.pppg++ <--> PppLink100M <--> R1.pppg++;
        R1.pppg++ <--> PppLink100M <--> R3.pppg++;
        R1.pppg++ <--> PppLink100M <--> R4.pppg++;
        R1.pppg++ <--> PppLink100M <--> R5.pppg++;
        switch2.ethg++ <--> Eth100M <--> R2.ethg++;
        switch4.ethg++ <--> Eth100M <--> R4.ethg++;
        switch5.ethg++ <--> Eth100M <--> R5.ethg++;
        R3.pppg++ <--> PppLink100M <--> R6.pppg++;
}


