package inet.examples.inet.ectNet;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.common.queue.FIFOQueue;
import inet.common.queue.IOutputQueue;
import inet.common.queue.REDDropper;
import inet.node.inet.StandardHost;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth10M;
import inet.node.inet.Router;
import ned.DatarateChannel;

module REDQueue like IOutputQueue
{
    gates:
        input in;
        output out;

    submodules:
        red: REDDropper {
            numGates = 1;
            wq = default(0.002);
            minths = default("30");
            maxths = default("90");
            maxps = default("0.10");
            pkrates = default("8333.333");
        }
        fifo: FIFOQueue;
    connections:
        in --> red.in[0];
        red.out[0] --> fifo.in++;
        fifo.out --> out;
}


network testNetwork
{
    // Network Components
    submodules:
        configurator: IPv4NetworkConfigurator {
            @display("p=50,190");
        }
        visualizer: IntegratedVisualizer {
            @display("p=19,190");
        }

        // Routers
        Router1: Router {
            parameters:
                @display("p=300,150,c,150;i=abstract/router_l,gold");
            gates:
                pppg[2];
        }
        Router2: Router {
            parameters:
                @display("p=600,150,c,150;i=abstract/router_l,gold");
            gates:
                pppg[2];
        }
        Router3: Router {
            parameters:
                @display("p=1000,150,c,150;i=abstract/router_l,gold");
            gates:
                pppg[2];
        }
        Router4: Router {
            parameters:
                @display("p=1300,150,c,150;i=abstract/router_l,gold");
            gates:
                pppg[2];
        }

        client: StandardHost {
            parameters:
                forwarding = false;
                @display("p=50,50,c,200;i=device/laptop_l,green");
            gates:
                pppg[1];
        }
        srv: StandardHost {
            parameters:
                forwarding = false;
                @display("p=1550,50,c,200;i=device/server_l,green");
            gates:
                pppg[1];
        }

    connections:
        Router2.pppg++ <--> Eth10M { @display("ls=red,7"); } <--> Router3.pppg++;
        Router1.pppg++ <--> Eth100M { @display("ls=blue,5"); } <--> Router2.pppg++;
        Router4.pppg++ <--> Eth100M { @display("ls=blue,5"); } <--> Router3.pppg++;
        client.pppg++ <--> Eth100M { @display("ls=blue,5"); } <--> Router1.pppg++;
        srv.pppg++ <--> Eth100M { @display("ls=blue,5"); } <--> Router4.pppg++;
}
