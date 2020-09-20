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


network OSPF_Area_External extends OSPF_AreaTest
{
    @display("bgb=1151.665,517.31");
    submodules:
        N1: OspfLan {
            parameters:
                h = 1;
                @display("p=539.45123,389.04126");
        }
        N2: OspfLan {
            parameters:
                h = 1;
                @display("p=820.02374,389.04126");
        }

    connections:
        R3.ethg++ <--> Eth100M <--> N1.ethg++;
        R5.ethg++ <--> Eth100M <--> N2.ethg++;
}

