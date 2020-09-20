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


network Network
{
    @display("bgb=1766.2515,641.51587");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=491.625,81.225");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=756.675,81.225");
        }
        scenarioManager: ScenarioManager {
            @display("p=1045.2375,81.225");
        }
        host0: StandardHost {
            @display("p=98.325,200.925");
        }
        host1: StandardHost {
            @display("p=98.325,356.9625");
        }
        host2: StandardHost {
            @display("p=98.325,519.4125");
        }
        R1: Router {
            @display("p=545.0625,356.9625");
        }
        R2: Router {
            @display("p=989.6625,356.9625");
        }
        host3: StandardHost {
            @display("p=1545.4125,196.65");
        }
        host4: StandardHost {
            @display("p=1545.4125,356.9625");
        }
        host5: StandardHost {
            @display("p=1545.4125,517.275");
        }
        switch1: EtherSwitch {
            @display("p=320.625,356.9625");
        }
        switch2: EtherSwitch {
            @display("p=1295.325,352.6875");
        }
    connections:
        host2.ethg++ <--> Eth100M <--> switch1.ethg++;
        host1.ethg++ <--> Eth100M <--> switch1.ethg++;
        host0.ethg++ <--> Eth100M <--> switch1.ethg++;
        switch1.ethg++ <--> Eth100M <--> R1.ethg++;

        switch2.ethg++ <--> Eth100M <--> host3.ethg++;
        switch2.ethg++ <--> Eth100M <--> host4.ethg++;
        switch2.ethg++ <--> Eth100M <--> host5.ethg++;

        R1.pppg++ <--> PppLink100M <--> R2.pppg++;
        R2.ethg++ <--> PppLink100M <--> switch2.ethg++;
}

