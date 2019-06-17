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


network RipNetworkA
{
    @display("bgb=2101.0125,829.44995");
    @figure[simtime](type="simTimeText"; pos=1550,60; prefix="SimTime: "; font=,14);
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=757.8,76.30625");
        }
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer {
            @display("p=1023.5563,76.30625");
        }
        scenarioManager: ScenarioManager {
            @display("p=1310.3625,76.30625");
        }
        host0: StandardHost {
            @display("p=97.81249,375.59998;i=,red");
        }
        host1: StandardHost {
            @display("p=97.81249,532.1");
        }
        host2: StandardHost {
            @display("p=97.81249,694.46875");
        }
        host3: StandardHost {
            @display("p=1271.5625,334.51874");
        }
        router0: Router {
            @display("p=543.83746,532.1");
        }
        router2: Router {
            @display("p=1508.2687,532.1");
        }
        router1: Router {
            @display("p=925.3062,532.1");
        }
        host4: StandardHost {
            @display("p=1271.5625,692.51245");
        }
        host5: StandardHost {
            @display("p=1473.0562,389.29373");
        }
        host6: StandardHost {
            @display("p=1977.7687,373.64374;i=,blue");
        }
        host7: StandardHost {
            @display("p=1977.7687,532.1");
        }
        host8: StandardHost {
            @display("p=1977.7687,692.51245");
        }
        switch0: EtherSwitch {
            @display("p=318.86874,530.14374");
        }
        switch2: EtherSwitch {
            @display("p=1727.3687,530.14374");
        }
        switch1: EtherSwitch {
            @display("p=1271.5625,530.14374");
        }
        router3: Router {
            @display("p=1054.4187,334.51874");
        }
        router4: Router {
            @display("p=712.07495,334.51874");
        }
    connections:
        host2.ethg++ <--> Eth100M <--> switch0.ethg++;
        host1.ethg++ <--> Eth100M <--> switch0.ethg++;
        host0.ethg++ <--> Eth100M <--> switch0.ethg++;
        switch0.ethg++ <--> Eth100M <--> router0.ethg++;

        switch1.ethg++ <--> Eth100M <--> host3.ethg++;
        switch1.ethg++ <--> Eth100M <--> host4.ethg++;
        switch1.ethg++ <--> Eth100M <--> host5.ethg++;
        switch1.ethg++ <--> Eth100M <--> router2.ethg++;

        switch2.ethg++ <--> Eth100M <--> host6.ethg++;
        switch2.ethg++ <--> Eth100M <--> host7.ethg++;
        switch2.ethg++ <--> Eth100M <--> host8.ethg++;

        router2.ethg++ <--> Eth100M <--> switch2.ethg++;
        router0.ethg++ <--> Eth10M <--> router1.ethg++;
        router1.ethg++ <--> Eth10M <--> switch1.ethg++;
        router0.ethg++ <--> Eth100M <--> router4.ethg++;
        router4.ethg++ <--> Eth100M <--> router3.ethg++;
        router3.ethg++ <--> Eth100M <--> switch1.ethg++;
}

network RipNetworkB
{
    @display("bgb=2101.0125,829.44995");
    @figure[simtime](type="simTimeText"; pos=1550,60; prefix="SimTime: "; font=,14);
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=757.8,76.30625");
        }
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer {
            @display("p=1023.5563,76.30625");
        }
        scenarioManager: ScenarioManager {
            @display("p=1310.3625,76.30625");
        }
        host0: StandardHost {
            @display("p=97.81249,375.59998;i=,red");
        }
        host1: StandardHost {
            @display("p=97.81249,532.1");
        }
        host2: StandardHost {
            @display("p=97.81249,694.46875");
        }
        host3: StandardHost {
            @display("p=1271.5625,334.51874");
        }
        router0: Router {
            @display("p=543.83746,532.1");
        }
        router2: Router {
            @display("p=1508.2687,532.1");
        }
        router1: Router {
            @display("p=925.3062,532.1");
        }
        host4: StandardHost {
            @display("p=1271.5625,692.51245");
        }
        host5: StandardHost {
            @display("p=1473.0562,389.29373");
        }
        host6: StandardHost {
            @display("p=1977.7687,373.64374;i=,blue");
        }
        host7: StandardHost {
            @display("p=1977.7687,532.1");
        }
        host8: StandardHost {
            @display("p=1977.7687,692.51245");
        }
        switch0: EtherSwitch {
            @display("p=318.86874,530.14374");
        }
        switch2: EtherSwitch {
            @display("p=1727.3687,530.14374");
        }
        switch1: EtherSwitch {
            @display("p=1270.8938,470.99374");
        }
        router3: Router {
            @display("p=1054.4187,334.51874");
        }
        router4: Router {
            @display("p=712.07495,334.51874");
        }
    connections:
        host2.ethg++ <--> Eth100M <--> switch0.ethg++;
        host1.ethg++ <--> Eth100M <--> switch0.ethg++;
        host0.ethg++ <--> Eth100M <--> switch0.ethg++;
        switch0.ethg++ <--> Eth100M <--> router0.ethg++;

        switch1.ethg++ <--> Eth100M <--> host3.ethg++;
        switch1.ethg++ <--> Eth100M <--> host4.ethg++;
        switch1.ethg++ <--> Eth100M <--> host5.ethg++;
        switch1.ethg++ <--> Eth100M <--> router2.ethg++;

        switch2.ethg++ <--> Eth100M <--> host6.ethg++;
        switch2.ethg++ <--> Eth100M <--> host7.ethg++;
        switch2.ethg++ <--> Eth100M <--> host8.ethg++;

        router2.ethg++ <--> Eth100M <--> switch2.ethg++;
        router0.ethg++ <--> Eth10M <--> router1.ethg++;
        router1.ethg++ <--> Eth10M <--> switch1.ethg++;
        router0.ethg++ <--> Eth100M <--> router4.ethg++;
        router4.ethg++ <--> Eth100M <--> router3.ethg++;
        router3.ethg++ <--> Eth100M <--> switch1.ethg++;
}


network RipNetworkC
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


network RipNetworkD
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
