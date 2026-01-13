package inet.tutorials.ospf;

import inet.common.packet.recorder.PcapRecorder;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;
import ned.DatarateChannel;


network OSPF_Area_External_Forwarding
{
    @display("bgb=1064.7,561.6");
    types:
        channel PppLink100M extends DatarateChannel
        {
            delay = 5us;
            datarate = 100Mbps;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=618.8,49.399998");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=750.1,49.399998");
        }
        routingTableRecorder: RoutingTableRecorder {
            @display("p=330,49");
        }
        pcapRecorder: PcapRecorder {
            @display("p=479,49");
        }
        scenarioManager: ScenarioManager {
            @display("p=913.89996,49.399998");
        }
        host1: StandardHost {
            @display("p=92.299995,120.9");
        }
        host2: StandardHost {
            @display("p=92.299995,500.5");
        }
        R1: Router {
            @display("p=382.19998,210.59999");
        }
        R2: Router {
            @display("p=382.19998,390");
        }
        R3: Router {
            @display("p=531.7,299");
        }
        switch1: EthernetSwitch {
            @display("p=232.7,118.299995");
        }
        switch2: EthernetSwitch {
            @display("p=232.7,499.19998");
        }
        R4: Router {
            @display("p=683.8,299");
        }
        R5: Router {
            @display("p=845,410.8");
        }
        R6: Router {
            @display("p=845,183.3");
        }
        switch3: EthernetSwitch {
            @display("p=845,297.69998");
        }
        host3: StandardHost {
            @display("p=683.8,183.3");
        }

    connections:
        host1.ethg++ <--> Eth100M <--> switch1.ethg++;
        switch1.ethg++ <--> Eth100M <--> R1.ethg++;
        R1.pppg++ <--> PppLink100M <--> R3.pppg++;
        switch2.ethg++ <--> Eth100M <--> host2.ethg++;
        R1.pppg++ <--> PppLink100M <--> R2.pppg++;
        R2.pppg++ <--> PppLink100M <--> R3.pppg++;
        R2.ethg++ <--> Eth100M <--> switch2.ethg++;
        R5.ethg++ <--> Eth100M <--> switch3.ethg++;
        R3.pppg++ <--> PppLink100M <--> R4.pppg++;
        switch3.ethg++ <--> Eth100M <--> R4.ethg++;
        R6.ethg++ <--> Eth100M <--> switch3.ethg++;
        host3.ethg++ <--> Eth100M <--> R6.ethg++;
}


