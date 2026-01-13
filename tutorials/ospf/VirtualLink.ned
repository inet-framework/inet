package inet.tutorials.ospf;

import inet.common.packet.recorder.PcapRecorder;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;
import ned.DatarateChannel;


network VirtualLink
{
    @display("bgb=1529.2438,788.60626");

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
            @display("p=136.23125,197.63124");
        }
        routingTableRecorder: RoutingTableRecorder {
            @display("p=379,211");
        }
        pcapRecorder: PcapRecorder {
            @display("p=607,61");
        }
        scenarioManager: ScenarioManager {
            @display("p=379.9125,61.399998");
        }
        R1: Router {
            @display("p=176.525,401.01874");
        }
        R2: Router {
            @display("p=483.525,401.01874");
        }
        R3: Router {
            @display("p=1076.4187,401.01874");
        }
        R4: Router {
            @display("p=1377.6625,401.01874");
        }
        switch1: EthernetSwitch {
            @display("p=176.525,583.3");
        }
        switch2: EthernetSwitch {
            @display("p=483.525,583.3");
        }
        switch3: EthernetSwitch {
            @display("p=1076.4187,583.3");
        }
        switch4: EthernetSwitch {
            @display("p=1377.6625,583.3");
        }
        R5: Router {
            @display("p=757.90625,401.01874");
        }
        switch5: EthernetSwitch {
            @display("p=757.90625,583.3");
        }
        switch6: EthernetSwitch {
            @display("p=755.9875,209.14375");
        }
    connections:
        R2.pppg++ <--> PppLink100M <--> R1.pppg++;
        R3.pppg++ <--> PppLink100M <--> R4.pppg++;
        R1.ethg++ <--> Eth100M <--> switch1.ethg++;
        R2.ethg++ <--> Eth100M <--> switch2.ethg++;
        R3.ethg++ <--> Eth100M <--> switch3.ethg++;
        R4.ethg++ <--> Eth100M <--> switch4.ethg++;
        R5.ethg++ <--> Eth100M <--> switch6.ethg++;
        R5.ethg++ <--> Eth100M <--> switch5.ethg++;
        R2.pppg++ <--> PppLink100M <--> R5.pppg++;
        R5.pppg++ <--> PppLink100M <--> R3.pppg++;
}

