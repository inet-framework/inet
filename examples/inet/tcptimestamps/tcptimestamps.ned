//
// author: Thomas Reschka
//

package inet.examples.inet.tcptimestamps;

import ned.DatarateChannel;
import inet.nodes.inet.StandardHost;

network tcptimestamps {
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
    connections:
        client.pppg[0] <--> NormalPath <--> server.pppg[0];
}

channel NormalPath extends DatarateChannel {
    parameters:
        datarate = 100Mbps;
        delay = 0.5ms;
        per = 0;
        ber = 0;
}
