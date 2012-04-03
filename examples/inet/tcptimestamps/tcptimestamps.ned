//
// author: Thomas Reschka
//

package inet.examples.inet.tcptimestamps;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.StandardHost;
import ned.DatarateChannel;


network tcptimestamps
{
    parameters:
        @display("bgb=400,200");
    submodules:
        client: StandardHost {
            parameters:
                @display("p=50,100");
            gates:
                pppg[1];
        }
        server: StandardHost {
            parameters:
                @display("p=350,100;i=device/server");
            gates:
                pppg[1];
        }
        networkConfigurator: IPv4NetworkConfigurator {
            @display("p=187,34");
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
