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


network OSPF_AreaTest
{
    @display("bgb=1153.1,510.9");

    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=760.5,65");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=893.1,65");
        }
        scenarioManager: ScenarioManager {
            @display("p=1055.6,65");
        }
        host1: StandardHost {
            @display("p=102.7,67.6");
        }
        host2: StandardHost {
            @display("p=92.299995,445.9");
        }
        R1: Router {
            @display("p=392.6,157.3");
        }
        R2: Router {
            @display("p=392.6,336.69998");
        }
        R3: Router {
            @display("p=542.1,245.7");
        }
        switch1: EtherSwitch {
            @display("p=243.09999,66.299995");
        }
        switch2: EtherSwitch {
            @display("p=243.09999,445.9");
        }
        host3: StandardHost {
            @display("p=1085.5,245.7");
        }
        R4: Router {
            @display("p=678.6,245.7");
        }
        R5: Router {
            @display("p=821.6,245.7");
        }
        switch3: EtherSwitch {
            @display("p=955.5,244.4");
        }

    connections:
        host1.ethg++ <--> Eth100M <--> switch1.ethg++;
        switch1.ethg++ <--> Eth100M <--> R1.ethg++;
        R1.pppg++ <--> PppLink100M <--> R3.pppg++;
        switch2.ethg++ <--> Eth100M <--> host2.ethg++;
        R1.pppg++ <--> PppLink100M <--> R2.pppg++;
        R2.pppg++ <--> PppLink100M <--> R3.pppg++;
        R2.ethg++ <--> Eth100M <--> switch2.ethg++;
        R4.pppg++ <--> PppLink100M <--> R5.pppg++;
        R5.ethg++ <--> Eth100M <--> switch3.ethg++;
        switch3.ethg++ <--> Eth100M <--> host3.ethg++;
        R3.pppg++ <--> PppLink100M <--> R4.pppg++;
}

