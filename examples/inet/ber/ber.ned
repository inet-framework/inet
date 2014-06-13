//
// author: Thomas Reschka
//

package inet.examples.inet.ber;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.StandardHost;


network ber
{
    parameters:
        double n;
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
        configurator: IPv4NetworkConfigurator {
            @display("p=199,34");
        }
    connections:
        client.pppg[0] <--> {  datarate = 10Mbps; delay = 0.565us; per = 0; ber = n; } <--> server.pppg[0];
}
