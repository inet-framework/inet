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


network OSPF_Suboptimal
{
    @display("bgb=772,445");

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
            @display("p=64.128,139.278");
        }
        R2: Router {
            @display("p=230.45999,139.278");
        }
        R3: Router {
            @display("p=559.11597,139.278");
        }
        R4: Router {
            @display("p=323.646,242.484");
        }
        R5: Router {
            @display("p=474.948,242.484");
        }
        switch4: EtherSwitch {
            @display("p=64.128,386.772");
        }
        switch3: EtherSwitch {
            @display("p=474.948,330.66");
        }
        R6: Router {
            @display("p=230.45999,387.774");
        }
        R7: Router {
            @display("p=559.11597,387.774");
        }
        switch1: EtherSwitch {
            @display("p=708,138");
        }
        switch2: EtherSwitch {
            @display("p=323.646,330.66");
        }
        host0: StandardHost {
            @display("p=65,246");
        }
        host6: StandardHost {
            @display("p=708,387");
        }
    connections:
        R2.ethg++ <--> Eth100M <--> R1.ethg++;
        R2.ethg++ <--> Eth100M <--> R3.ethg++;
        R2.ethg++ <--> Eth100M <--> R6.ethg++;
        R2.ethg++ <--> Eth100M <--> R4.ethg++;
        R4.ethg++ <--> Eth100M <--> R5.ethg++;
        R5.ethg++ <--> Eth100M <--> R3.ethg++;
        R3.ethg++ <--> Eth100M <--> R7.ethg++;
        R7.ethg++ <--> Eth100M <--> R6.ethg++;
        switch4.ethg++ <--> Eth100M <--> R6.ethg++;
        switch1.ethg++ <--> Eth100M <--> R3.ethg++;
        switch3.ethg++ <--> Eth100M <--> R5.ethg++;
        switch2.ethg++ <--> Eth100M <--> R4.ethg++;
        host0.ethg++ <--> Eth100M <--> R1.ethg++;
        host6.ethg++ <--> Eth100M <--> R7.ethg++;
}
