//
// author: Thomas Reschka
//

package inet.examples.inet.tcptimestamps;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network tcptimestamps
{
    parameters:
    submodules:
        client: StandardHost {
            parameters:
                @display("p=250,100");
            gates:
                pppg[1];
        }
        server: StandardHost {
            parameters:
                @display("p=400,100;i=device/server");
            gates:
                pppg[1];
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
    connections:
        client.pppg[0] <--> NormalPath <--> server.pppg[0];
}

channel NormalPath extends DatarateChannel
{
    parameters:
        datarate = 100Mbps;
        delay = 0.5ms;
        per = 0;
        ber = 0;
}
