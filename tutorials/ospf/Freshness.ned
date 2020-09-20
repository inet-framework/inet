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

network Freshness
{
    @display("bgb=1154.256,1107.552");

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
            @display("p=197.936,135.664");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=464.816,135.664");
        }
        scenarioManager: ScenarioManager {
            @display("p=749.48804,135.664");
        }
        R1: Router {
            @display("p=998.57605,535.984");
        }
        R5: Router {
            @display("p=749.48804,760.60803");
        }
        R6: Router {
            @display("p=998.57605,760.60803");
        }
        switch4: EtherSwitch {
            @display("p=449.24802,342.496");
        }
        switch5: EtherSwitch {
            @display("p=449.24802,962.992");
        }
        R3: Router {
            @display("p=449.24802,535.984");
        }
        R2: Router {
            @display("p=749.48804,535.984");
        }

        R4: Router {
            @display("p=449.24802,760.60803");
        }
        switch6: EtherSwitch {
            @display("p=749.48804,962.992");
        }
        switch7: EtherSwitch {
            @display("p=998.57605,962.992");
        }
        switch3: EtherSwitch {
            @display("p=749.48804,342.496");
        }
        switch2: EtherSwitch {
            @display("p=998.57605,342.496");
        }
        R7: Router {
            @display("p=171.248,760.60803");
        }
        switch1: EtherSwitch {
            @display("p=171.248,962.992");
        }
    connections:
        R1.pppg++ <--> PppLink100M <--> R2.pppg++;
        R2.pppg++ <--> PppLink100M <--> R3.pppg++;
        R4.pppg++ <--> PppLink100M <--> R5.pppg++;
        R5.pppg++ <--> PppLink100M <--> R6.pppg++;
        R6.pppg++ <--> PppLink100M <--> R1.pppg++;
        R3.pppg++ <--> PppLink100M <--> R4.pppg++;
        R3.ethg++ <--> Eth100M <--> switch4.ethg++;
        R4.ethg++ <--> Eth100M <--> switch5.ethg++;
        R5.ethg++ <--> Eth100M <--> switch6.ethg++;
        R6.ethg++ <--> Eth100M <--> switch7.ethg++;
        R2.ethg++ <--> Eth100M <--> switch3.ethg++;
        switch2.ethg++ <--> Eth100M <--> R1.ethg++;
        R7.ethg++ <--> Eth100M <--> switch1.ethg++;
        R7.pppg++ <--> PppLink100M <--> R4.pppg++;
}


