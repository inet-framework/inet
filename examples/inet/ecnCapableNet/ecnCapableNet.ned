package inet.examples.inet.ecnCapableNet;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.node.ethernet.Eth1G;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth10M;


network EcnCapableNet
{     
    submodules:
        configurator: IPv4NetworkConfigurator {
        }
        rte[2]: Router {
            parameters:
                @display("is=vs");
        }
        bottleneck_rte: Router {
            parameters:
                @display("is=vs");
        }
        cli: StandardHost {
            parameters:
                @display("i=device/laptop_s");
        }
        srv: StandardHost {
            parameters:
                @display("i=device/server_s");
        }
        visualizer: IntegratedVisualizer {
            @display("p=267,37");
        }
        
    connections:
        cli.pppg++ <--> Eth1G <--> bottleneck_rte.pppg++;
        bottleneck_rte.pppg++ <--> Eth10M <--> rte[0].pppg++;
        rte[0].pppg++ <--> Eth100M <--> rte[1].pppg++;
        srv.pppg++ <--> Eth100M <--> rte[1].pppg++;
}

