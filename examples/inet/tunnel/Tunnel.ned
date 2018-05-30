package inet.examples.inet.tunnel;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth1G;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;


network Tunnel
{
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        client: StandardHost {
            parameters:
                @display("p=250,100");
        }
        inlet: Router {
            parameters:
                @display("p=400,100");
        }
        router: Router {
            parameters:
                @display("p=550,100");
        }
        outlet: Router {
            parameters:
                @display("p=700,100");
        }
        server: StandardHost {
            parameters:
                @display("p=850,100");
        }
    connections:
        client.ethg++ <--> Eth1G <--> inlet.ethg++;
        inlet.pppg++ <--> {  delay = 1us; datarate = 1Gbps; } <--> router.pppg++;
        router.pppg++ <--> {  delay = 1us; datarate = 1Gbps; } <--> outlet.pppg++;
        outlet.ethg++ <--> Eth1G <--> server.ethg++;
}

