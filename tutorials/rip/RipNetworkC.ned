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


network RipNetworkC
{
    @display("bgb=2066.82,862.47003");
    @figure[simtime](type="simTimeText"; pos=1550,60; prefix="SimTime: "; font=,14);
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=683.76,77.700005");
        }
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer {
            @display("p=936.72504,78.9375");
        }
        scenarioManager: ScenarioManager {
            @display("p=1210.375,78.9375");
        }
        host0: StandardHost {
            @display("p=116.55,271.95");
        }
        host1: StandardHost {
            @display("p=116.55,489.51");
        }
        host2: StandardHost {
            @display("p=116.55,717.43");
        }
        router0: Router {
            @display("p=986.79004,271.95");
        }
        router1: Router {
            @display("p=1289.8201,489.51");
        }
        host3: StandardHost {
            @display("p=1924.37,271.95");
        }
        host4: StandardHost {
            @display("p=1924.37,489.51");
        }
        host5: StandardHost {
            @display("p=1924.37,717.43");
        }
        switch0: EtherSwitch {
            @display("p=406.63,486.92");
        }
        switch1: EtherSwitch {
            @display("p=1605.8,486.92");
        }
        router2: Router {
            @display("p=704.48,489.51");
        }
        router3: Router {
            @display("p=986.79004,717.43");
        }
    connections:
        host2.ethg++ <--> Eth100M <--> switch0.ethg++;
        host1.ethg++ <--> Eth100M <--> switch0.ethg++;
        host0.ethg++ <--> Eth100M <--> switch0.ethg++;
        switch0.ethg++ <--> Eth100M <--> router2.ethg++;

        switch1.ethg++ <--> Eth100M <--> host3.ethg++;
        switch1.ethg++ <--> Eth100M <--> host4.ethg++;
        switch1.ethg++ <--> Eth100M <--> host5.ethg++;

        router0.ethg++ <--> Eth100M <--> router1.ethg++;
        router1.ethg++ <--> Eth100M <--> switch1.ethg++;
        router2.ethg++ <--> Eth100M <--> router3.ethg++;
        router2.ethg++ <--> Eth100M <--> router0.ethg++;
        router3.ethg++ <--> Eth100M <--> router0.ethg++;
        router3.ethg++ <--> Eth100M <--> router1.ethg++;
}
