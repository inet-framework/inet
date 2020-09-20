package inet.tutorials.rip;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth10M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;


network RipNetworkB
{
    @display("bgb=2068.1626,705.175");
    @figure[simtime](type="simTimeText"; pos=1550,60; prefix="SimTime: "; font=,14);
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=694.65,78.9375");
        }
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer {
            @display("p=936.72504,78.9375");
        }
        scenarioManager: ScenarioManager {
            @display("p=1210.375,78.9375");
        }
        host0: StandardHost {
            @display("p=118.40625,131.5625");
        }
        host1: StandardHost {
            @display("p=118.40625,347.325");
        }
        host2: StandardHost {
            @display("p=118.40625,576.2438");
        }
        router0: Router {
            @display("p=765.6938,347.325");
        }
        router1: Router {
            @display("p=1263,347.325");
        }
        host3: StandardHost {
            @display("p=1926.0751,131.5625");
        }
        host4: StandardHost {
            @display("p=1926.0751,347.325");
        }
        host5: StandardHost {
            @display("p=1926.0751,576.2438");
        }
        switch0: EtherSwitch {
            @display("p=407.84375,344.69376");
        }
        switch1: EtherSwitch {
            @display("p=1607.6937,344.69376");
        }
    connections:
        host2.ethg++ <--> Eth100M <--> switch0.ethg++;
        host1.ethg++ <--> Eth100M <--> switch0.ethg++;
        host0.ethg++ <--> Eth100M <--> switch0.ethg++;
        switch0.ethg++ <--> Eth100M <--> router0.ethg++;

        switch1.ethg++ <--> Eth100M <--> host3.ethg++;
        switch1.ethg++ <--> Eth100M <--> host4.ethg++;
        switch1.ethg++ <--> Eth100M <--> host5.ethg++;

        router0.ethg++ <--> Eth100M <--> router1.ethg++;
        router1.ethg++ <--> Eth100M <--> switch1.ethg++;
}
