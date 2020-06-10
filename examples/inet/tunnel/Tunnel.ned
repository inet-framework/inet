package inet.examples.inet.tunnel;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EtherLink;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network Tunnel
{
    parameters:
        *.eth[*].bitrate = 1 Gbps;
    types:
        channel ppplink extends DatarateChannel
        {
            delay = 1 us;
            datarate = 1 Gbps;
        }
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
        client.ethg++ <--> EtherLink <--> inlet.ethg++;
        inlet.pppg++ <--> ppplink <--> router.pppg++;
        router.pppg++ <--> ppplink <--> outlet.pppg++;
        outlet.ethg++ <--> EtherLink <--> server.ethg++;
}

