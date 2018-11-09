//
// author: Thomas Reschka
//

package inet.examples.inet.tcpwindowscale;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;


network tcpwindowscale
{
    parameters:
    types:
        channel LFNPath extends ThruputMeteringChannel
        {
            parameters:
                datarate = 1Gbps;
                delay = 565ms;
                per = 0;
                ber = 0;
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
        client.pppg[0] <--> LFNPath <--> server.pppg[0];
}

