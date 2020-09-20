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


network OSPF_Stub extends OSPF_AreaTest
{
    @display("bgb=1151.665,517.31");
    submodules:
        N1: OspfLan {
            parameters:
                h = 1;
                @display("p=470.66248,443.23123");
        }
        N2: OspfLan {
            parameters:
                h = 1;
                @display("p=677.1187,443.23123");
        }
        N3: OspfLan {
            parameters:
                h = 1;
                @display("p=847.4812,444.675");
        }

    connections:
        R2.ethg++ <--> Eth100M <--> N1.ethg++;
        R3.ethg++ <--> Eth100M <--> N2.ethg++;
        R4.ethg++ <--> Eth100M <--> N3.ethg++;
}


