
package inet.examples.ospfv2.areas;

import ned.DatarateChannel;
import inet.nodes.ospfv2.OSPFRouter;
import inet.nodes.inet.StandardHost;
import inet.linklayer.ethernet.EtherHub;


module OneNetArea
{
    parameters:
        @display("p=10,10;b=404,108");
    gates:
        inout ethg[];
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        H3: StandardHost {
            parameters:
                @display("p=372,72;i=device/laptop");
            gates:
                ethg[1];
        }
        N3: EtherHub {
            parameters:
                @display("p=264,72");
            gates:
                ethg[2];
        }
        R5: OSPFRouter {
            parameters:
                @display("p=160,72");
            gates:
                ethg[2];
        }
        R4: OSPFRouter {
            parameters:
                @display("p=64,72");
            gates:
                ethg[2];
        }
    connections:
        R4.ethg[0] <--> C <--> R5.ethg[1];
        R5.ethg[0] <--> C <--> N3.ethg[1];
        N3.ethg[0] <--> C <--> H3.ethg[0];

        // towards other areas
        ethg[0] <--> R4.ethg[1];
}

