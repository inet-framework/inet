package inet.tutorials.ospf;

import inet.common.packet.recorder.PcapRecorder;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;


network OSPF_LoopAvoidance
{
    @display("bgb=692,488");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=62.18875,63.635002");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=195.24374,63.635002");
        }
        routingTableRecorder: RoutingTableRecorder {
            @display("p=483,63");
        }
        pcapRecorder: PcapRecorder {
            @display("p=628,63");
        }
        scenarioManager: ScenarioManager {
            @display("p=357.22375,63.635002");
        }
        R1: Router {
            @display("p=218,179");
        }
        R2: Router {
            @display("p=474,179");
        }
        R3: Router {
            @display("p=349,297");
        }
        R4: Router {
            @display("p=474,411");
        }
        R5: Router {
            @display("p=218,411");
        }
        switch1: EthernetSwitch {
            @display("p=72,177");
        }
        switch2: EthernetSwitch {
            @display("p=619,177");
        }
        switch3: EthernetSwitch {
            @display("p=250,296");
        }
        switch4: EthernetSwitch {
            @display("p=619,410");
        }
        switch5: EthernetSwitch {
            @display("p=72,410");
        }
        switch6: EthernetSwitch {
            @display("p=349,410");
        }

    connections:
        switch1.ethg++ <--> Eth100M <--> R1.ethg++;
        R2.ethg++ <--> Eth100M <--> R3.ethg++;
        R2.ethg++ <--> Eth100M <--> switch2.ethg++;
        R5.ethg++ <--> Eth100M <--> switch5.ethg++;
        R3.ethg++ <--> Eth100M <--> R4.ethg++;
        R3.ethg++ <--> Eth100M <--> R5.ethg++;
        R5.ethg++ <--> Eth100M <--> R1.ethg++;
        R4.ethg++ <--> Eth100M <--> R2.ethg++;
        switch4.ethg++ <--> Eth100M <--> R4.ethg++;
        R1.ethg++ <--> Eth100M <--> R2.ethg++;
        switch3.ethg++ <--> Eth100M <--> R3.ethg++;
        switch6.ethg++ <--> Eth100M <--> R3.ethg++;
}


