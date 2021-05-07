//
// author: Thomas Reschka
//

package inet.examples.inet.ber;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;


network ber
{
    parameters:
        double n;
    types:
        channel C extends ThruputMeteringChannel {
            parameters:
                datarate = 10Mbps;
                delay = 0.565us;
                per = parent.n;
                thruputDisplayFormat = default("p b u");
        }
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
        client.pppg[0] <--> C <--> server.pppg[0];
}

