//
// author: Thomas Reschka
//

package inet.examples.inet.ber;

import inet.nodes.inet.StandardHost;
import inet.nodes.inet.StandardHostWithDLThruputMeter;
import ned.DatarateChannel;

network ber {
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
        server: StandardHostWithDLThruputMeter {
            parameters:
                @display("p=350,100;i=device/server");
            gates:
                pppg[1];
        }
    connections:
        client.pppg[0] <--> { datarate = 10Mbps; delay = 0.565us; per = 0; ber = n; } <--> server.pppg[0];
}
