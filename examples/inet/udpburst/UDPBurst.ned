package inet.examples.inet.udpburst;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network UDPBurst
{
    parameters:
        int numhosts;
    types:
        channel C extends DatarateChannel
        {
            datarate = 1Mbps;
            delay = 0.1us;
        }
    submodules:
        host[numhosts]: StandardHost {
            parameters:
                @display("i=device/pc3");
        }
        sink: StandardHost {
            parameters:
                @display("p=178,247;i=device/pc2");
        }
        router: Router {
            parameters:
                @display("p=178,153");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=62,119");
        }
    connections:
        for i=0..numhosts-1 {
            host[i].pppg++ <--> C <--> router.pppg++;
        }
        sink.pppg++ <--> C <--> router.pppg++;
}

