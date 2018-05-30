//
// author: Thomas Reschka
//

package inet.examples.inet.tcpsack;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network tcpsack
{
    parameters:
    submodules:
        router: Router {
            parameters:
                @display("p=400,100;i=abstract/router");
            gates:
                pppg[2];
        }
        client: StandardHost {
            parameters:
                @display("p=250,100");
            gates:
                pppg[1];
        }
        server: StandardHost {
            parameters:
                @display("p=550,100;i=device/server");
            gates:
                pppg[1];
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
    connections:
        router.pppg[0] <--> NormalPath <--> client.pppg[0];
        router.pppg[1] <--> NormalPath <--> server.pppg[0];
}

channel NormalPath extends DatarateChannel
{
    parameters:
        datarate = 100Mbps;
        delay = 0.0565us; // 10m
        per = 0;
        ber = 0;
}
